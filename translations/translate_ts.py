# -*- coding: utf-8 -*-
"""Перевод Qt .ts через локальную Ollama.

Запуск (аргументов нет, всё задаётся константами ниже):

    python translations/translate_ts.py

Читает translations/QtLanguage_ru.ts и пишет translations/QtLanguage_<код>.ts.
Файлы .qm не собирает: на машине без Qt их всё равно не из чего сделать.
"""

from __future__ import annotations

import json
import os
import re
import sys
import urllib.error
import urllib.request
import xml.etree.ElementTree as ET
from collections import defaultdict
from pathlib import Path

# --- настройки ---------------------------------------------------------------

# Адрес локальной Ollama.
OLLAMA_HOST = "http://127.0.0.1:11434"

# Точное имя модели из `ollama list`, например "qwen2.5:14b".
OLLAMA_MODEL = "qwen2.5:14b"

# Коды языков Qt. Файл: translations/QtLanguage_<код>.ts
# Примеры: "de", "fr", "es", "zh_CN", "ja".
TARGET_LANGUAGES = [
    # "de",
]

# Сколько строк отправлять в одном запросе.
BATCH_SIZE = 20

# False — не трогать уже заполненный перевод той же строки.
# True — перевести каталог заново.
RETRANSLATE_EXISTING = False

# 0 — все недостающие строки. Положительное число обрезает этот запуск.
LIMIT = 0

# Общие термины. Уходят в каждый запрос.
GLOSSARY = (
    "Autogen — automatic generation of planet parameters (автогенерация параметров).",
    "Biom — biome. The English interface spells it Biom.",
    "seed — random seed of the generator, not a plant seed (зерно случайности).",
    "shelf — continental shelf (континентальный шельф).",
)

# Температура модели. Ниже — стабильнее формулировки.
TEMPERATURE = 0.2

# Пауза загрузки модели между батчами, чтобы Ollama не выгружала её.
KEEP_ALIVE = "30m"

# Сколько раз повторять запрос при обрыве сети или невалидном JSON.
JSON_RETRIES = 3

# Секунды на один ответ модели.
REQUEST_TIMEOUT = 300

# -----------------------------------------------------------------------------

ROOT = Path(__file__).resolve().parent
SOURCE_TS = ROOT / "QtLanguage_ru.ts"
ACHIEVEMENTS_JSON = ROOT.parent / "res" / "txt_files" / "achievements.json"

LANGUAGE_NAMES = {
    "de": "German",
    "fr": "French",
    "es": "Spanish",
    "it": "Italian",
    "pt": "Portuguese",
    "pt_BR": "Brazilian Portuguese",
    "zh_CN": "Simplified Chinese",
    "zh_TW": "Traditional Chinese",
    "ja": "Japanese",
    "ko": "Korean",
    "pl": "Polish",
    "uk": "Ukrainian",
    "tr": "Turkish",
    "nl": "Dutch",
    "sv": "Swedish",
    "cs": "Czech",
    "fi": "Finnish",
    "ar": "Arabic",
}

_PLACEHOLDER_RE = re.compile(r"%%|%L?\d+")
_SPACE_RUN_RE = re.compile(r"[ \t]{2,}")
_DEAD_TRANSLATION = ("obsolete", "vanished")


class Message:
    def __init__(
        self,
        context,
        source,
        comment,
        extracomment,
        russian,
        locations,
    ):
        self.context = context
        self.source = source
        self.comment = comment
        self.extracomment = extracomment
        self.russian = russian
        self.locations = locations
        self.translation = ""
        self.translation_type = "unfinished"
        self.hint = ""

    def key(self):
        return (self.context, self.source, self.comment)


def emit(text):
    print(text, flush=True)


def language_name(code):
    if code in LANGUAGE_NAMES:
        return LANGUAGE_NAMES[code]
    return "the language with Qt code " + code


def edge_spaces(text):
    lead_len = len(text) - len(text.lstrip(" \t"))
    trail_len = len(text) - len(text.rstrip(" \t"))
    trail = text[len(text) - trail_len :] if trail_len else ""
    return text[:lead_len], trail


def translation_problem(source, translated):
    """Пустая строка, если перевод сохранил плейсхолдеры и пробелы."""
    if translated == "":
        return "пустой перевод"
    if (
        _PLACEHOLDER_RE.findall(source) != _PLACEHOLDER_RE.findall(translated)
        or source.count("%") != translated.count("%")
    ):
        return "плейсхолдеры не совпали"
    if source.count("\n") != translated.count("\n"):
        return "число переводов строк не совпало"
    if edge_spaces(source) != edge_spaces(translated):
        return "ведущие или хвостовые пробелы не совпали"
    if _SPACE_RUN_RE.findall(source) != _SPACE_RUN_RE.findall(translated):
        return "пробелы выравнивания не совпали"
    return ""


def xml_text(text):
    return (
        text.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
    )


def xml_attr(text):
    return xml_text(text).replace('"', "&quot;")


def child_text(elem, tag):
    node = elem.find(tag)
    if node is None or node.text is None:
        return ""
    return node.text


def parse_ts(path):
    """Живые сообщения по контекстам. obsolete и vanished пропускаются."""
    root = ET.parse(path).getroot()
    contexts = []
    for ctx in root.findall("context"):
        name = child_text(ctx, "name")
        messages = []
        for node in ctx.findall("message"):
            trans = node.find("translation")
            trans_type = ""
            trans_text = ""
            if trans is not None:
                trans_type = trans.get("type") or ""
                trans_text = trans.text or ""
            if trans_type in _DEAD_TRANSLATION:
                continue
            locations = []
            for loc in node.findall("location"):
                locations.append((loc.get("filename") or "", loc.get("line") or ""))
            message = Message(
                context=name,
                source=child_text(node, "source"),
                comment=child_text(node, "comment"),
                extracomment=child_text(node, "extracomment"),
                russian=trans_text,
                locations=locations,
            )
            message.translation = trans_text
            message.translation_type = trans_type
            messages.append(message)
        contexts.append((name, messages))
    return contexts


def load_achievement_hints(path):
    """Английский title/description -> русские _comment, без дублей."""
    with path.open(encoding="utf-8-sig") as handle:
        data = json.load(handle)
    hints = defaultdict(list)
    for entry in data.get("achievements", []):
        note = (entry.get("_comment") or "").strip()
        if not note:
            continue
        for field in ("title", "description"):
            text = entry.get(field) or ""
            if text and note not in hints[text]:
                hints[text].append(note)
    return hints


def apply_hints(contexts, achievement_hints):
    for _name, messages in contexts:
        for message in messages:
            parts = []
            if message.extracomment:
                parts.append(message.extracomment)
            if message.context == "Achievements":
                for note in achievement_hints.get(message.source, []):
                    if note not in parts:
                        parts.append(note)
            message.hint = "\n".join(parts)


def flatten(contexts):
    messages = []
    for _name, group in contexts:
        messages.extend(group)
    return messages


def take_existing(contexts, existing_contexts):
    buckets = defaultdict(list)
    for message in flatten(existing_contexts):
        buckets[message.key()].append((message.translation, message.translation_type))
    for message in flatten(contexts):
        message.translation = ""
        message.translation_type = "unfinished"
        bucket = buckets.get(message.key())
        if not bucket:
            continue
        text, trans_type = bucket.pop(0)
        if text != "" and not RETRANSLATE_EXISTING:
            message.translation = text
            message.translation_type = trans_type


def location_files(message):
    seen = []
    for filename, _line in message.locations:
        if filename and filename not in seen:
            seen.append(filename)
    return seen


def system_prompt(code):
    glossary = "\n".join("- " + line for line in GLOSSARY)
    return (
        "You translate the user interface of God of Pixels 3, a desktop program that generates planets.\n"
        "Strings are short labels, tooltips, planet facts, and achievement names. "
        "Some achievements are jokes or references. Keep the tone and do not explain the joke.\n"
        "\n"
        "Translate into " + language_name(code) + ".\n"
        "The English source is the text to translate. The Russian field is a second reference for the meaning. "
        "If English and Russian disagree, follow English.\n"
        "If a hint is present, use it to choose the meaning. Do not copy the hint into the translation.\n"
        "If disambiguation is present, follow it. "
        '"metal genitive" is the genitive form of the metal name. '
        '"metal adjective" is the adjective form of the metal name.\n'
        "\n"
        "Rules:\n"
        "- Keep placeholders exactly as in English and in the same order: %1, %2, %L1, and %% .\n"
        "- Keep the same number of newline characters.\n"
        "- Keep the same leading and trailing spaces.\n"
        "- Keep every run of two or more spaces exactly, including its length. "
        'These runs align columns, as in "seed        - %1".\n'
        "- If the Russian text left a name or a chemical symbol unchanged "
        "(God of Pixels, Au, U), leave it unchanged too.\n"
        "- Do not add quotes, labels, or comments around the translation.\n"
        "\n"
        "Glossary:\n"
        + glossary
        + "\n\n"
        'Return a JSON object and nothing else: {"items":[{"id":"<same id>","text":"<translation>"}]}\n'
        "Include every input id exactly once."
    )


def payload_items(batch):
    items = []
    for index, message in enumerate(batch):
        item = {
            "id": str(index),
            "context": message.context,
            "source": message.source,
            "russian": message.russian,
        }
        if message.comment:
            item["disambiguation"] = message.comment
        if message.hint:
            item["hint"] = message.hint
        files = location_files(message)
        if files:
            item["locations"] = files
        items.append(item)
    return items


def ollama_json(method, path, payload=None, timeout=30):
    url = OLLAMA_HOST.rstrip("/") + path
    data = None
    if payload is not None:
        data = json.dumps(payload).encode("utf-8")
    request = urllib.request.Request(
        url,
        data=data,
        headers={"Content-Type": "application/json"},
        method=method,
    )
    try:
        with urllib.request.urlopen(request, timeout=timeout) as response:
            raw = response.read().decode("utf-8")
    except urllib.error.URLError as exc:
        raise RuntimeError("нет соединения с Ollama (" + OLLAMA_HOST + "): " + str(exc)) from exc
    if not raw:
        return {}
    return json.loads(raw)


def ensure_model():
    try:
        data = ollama_json("GET", "/api/tags", timeout=10)
    except RuntimeError as exc:
        raise SystemExit(
            str(exc) + "\nЗапустите ollama serve и проверьте OLLAMA_HOST."
        ) from exc
    except json.JSONDecodeError as exc:
        raise SystemExit("Ollama ответила не JSON на /api/tags.") from exc
    names = []
    for model in data.get("models", []):
        name = model.get("name") or model.get("model") or ""
        if name:
            names.append(name)
    if OLLAMA_MODEL not in names:
        available = ", ".join(names) if names else "—"
        raise SystemExit(
            "Модели "
            + OLLAMA_MODEL
            + " нет в Ollama. Выполните: ollama pull "
            + OLLAMA_MODEL
            + "\nДоступны: "
            + available
        )


def parse_model_items(content):
    text = content.strip().lstrip("\ufeff")
    if text.startswith("```"):
        text = re.sub(r"^```(?:json)?\s*", "", text)
        text = re.sub(r"\s*```$", "", text)
    data = json.loads(text)
    if isinstance(data, list):
        items = data
    elif isinstance(data, dict):
        items = data.get("items")
        if items is None:
            items = data.get("translations")
        if items is None:
            items = data.get("messages")
        if items is None and "id" in data:
            items = [data]
        if not isinstance(items, list):
            raise ValueError("в ответе нет списка items")
    else:
        raise ValueError("ответ не JSON-объект")
    found = {}
    for item in items:
        if not isinstance(item, dict) or "id" not in item:
            continue
        value = item.get("text")
        if value is None:
            value = item.get("translation")
        if value is None:
            continue
        found[str(item["id"])] = str(value)
    return found


def request_translations(code, batch, correction):
    user = json.dumps({"items": payload_items(batch)}, ensure_ascii=False, indent=2)
    if correction:
        user = correction + "\n\n" + user
    body = {
        "model": OLLAMA_MODEL,
        "stream": False,
        "format": "json",
        "keep_alive": KEEP_ALIVE,
        "options": {"temperature": TEMPERATURE},
        "messages": [
            {"role": "system", "content": system_prompt(code)},
            {"role": "user", "content": user},
        ],
    }
    last_error = None
    for attempt in range(1, JSON_RETRIES + 1):
        try:
            data = ollama_json("POST", "/api/chat", body, timeout=REQUEST_TIMEOUT)
            content = data.get("message", {}).get("content", "")
            if not content:
                raise ValueError("пустой ответ модели")
            return parse_model_items(content)
        except (RuntimeError, ValueError, json.JSONDecodeError, KeyError) as exc:
            last_error = exc
            emit("Попытка " + str(attempt) + " не удалась: " + str(exc))
    raise RuntimeError(str(last_error))


def accept_translation(message, text):
    problem = translation_problem(message.source, text)
    if problem:
        return problem
    message.translation = text
    message.translation_type = "unfinished"
    return ""


def mark_failed(message, reason):
    message.translation = ""
    message.translation_type = "unfinished"
    emit("Без перевода [" + message.context + "] " + message.source + ": " + reason)


def translate_batch(code, batch, correction, allow_split):
    try:
        found = request_translations(code, batch, correction)
    except RuntimeError as exc:
        if allow_split and len(batch) > 1:
            emit("Батч не разобран, перевожу по одной строке.")
            for message in batch:
                translate_batch(code, [message], "", True)
            return
        for message in batch:
            mark_failed(message, str(exc))
        return

    failed = []
    for index, message in enumerate(batch):
        text = found.get(str(index))
        if text is None:
            failed.append((message, "в ответе нет этой строки"))
            continue
        text = text.replace("\r\n", "\n").replace("\r", "\n")
        problem = accept_translation(message, text)
        if problem:
            failed.append((message, problem))
    if not failed:
        return
    if not allow_split:
        for message, reason in failed:
            mark_failed(message, reason)
        return
    for message, reason in failed:
        note = (
            "The previous translation was rejected: "
            + reason
            + ". Fix only that. Keep placeholders and space runs identical to the English source."
        )
        translate_batch(code, [message], note, False)


def render_ts(language, contexts):
    lines = [
        '<?xml version="1.0" encoding="utf-8"?>',
        "<!DOCTYPE TS>",
        '<TS version="2.1" language="' + xml_attr(language) + '">',
    ]
    for name, messages in contexts:
        lines.append("<context>")
        lines.append("    <name>" + xml_text(name) + "</name>")
        for message in messages:
            lines.append("    <message>")
            for filename, line_no in message.locations:
                attrs = ' filename="' + xml_attr(filename) + '"'
                if line_no:
                    attrs += ' line="' + xml_attr(line_no) + '"'
                lines.append("        <location" + attrs + "/>")
            lines.append("        <source>" + xml_text(message.source) + "</source>")
            if message.comment:
                lines.append("        <comment>" + xml_text(message.comment) + "</comment>")
            if message.hint:
                lines.append(
                    "        <extracomment>" + xml_text(message.hint) + "</extracomment>"
                )
            type_attr = ""
            if message.translation_type:
                type_attr = ' type="' + xml_attr(message.translation_type) + '"'
            lines.append(
                "        <translation"
                + type_attr
                + ">"
                + xml_text(message.translation)
                + "</translation>"
            )
            lines.append("    </message>")
        lines.append("</context>")
    lines.append("</TS>")
    lines.append("")
    return "\n".join(lines)


def write_ts(path, language, contexts):
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(render_ts(language, contexts), encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def chunks(items, size):
    if size < 1:
        raise SystemExit("BATCH_SIZE должен быть больше нуля.")
    for start in range(0, len(items), size):
        yield items[start : start + size]


def translate_language(code, contexts):
    output = ROOT / ("QtLanguage_" + code + ".ts")
    if output.is_file():
        take_existing(contexts, parse_ts(output))
    else:
        for message in flatten(contexts):
            message.translation = ""
            message.translation_type = "unfinished"

    pending = [message for message in flatten(contexts) if message.translation == ""]
    if LIMIT > 0:
        pending = pending[:LIMIT]
    kept = sum(1 for message in flatten(contexts) if message.translation != "")
    emit(
        code
        + ": уже есть "
        + str(kept)
        + ", к переводу "
        + str(len(pending))
        + ", файл "
        + output.name
    )
    batches = list(chunks(pending, BATCH_SIZE))
    for index, batch in enumerate(batches, start=1):
        emit(code + ": батч " + str(index) + "/" + str(len(batches)) + ", строк " + str(len(batch)))
        translate_batch(code, batch, "", True)
        write_ts(output, code, contexts)
    if not batches:
        write_ts(output, code, contexts)
    missing = sum(1 for message in flatten(contexts) if message.translation == "")
    done = sum(1 for message in flatten(contexts) if message.translation != "")
    emit(
        code
        + ": готово, с переводом "
        + str(done)
        + ", без перевода "
        + str(missing)
        + ". "
        + str(output)
    )


def prepare_catalog():
    if not SOURCE_TS.is_file():
        raise SystemExit("Нет файла " + str(SOURCE_TS))
    if not ACHIEVEMENTS_JSON.is_file():
        raise SystemExit("Нет файла " + str(ACHIEVEMENTS_JSON))
    contexts = parse_ts(SOURCE_TS)
    for _name, messages in contexts:
        for message in messages:
            message.russian = message.translation
            message.translation = ""
            message.translation_type = "unfinished"
    apply_hints(contexts, load_achievement_hints(ACHIEVEMENTS_JSON))
    return contexts


def main():
    try:
        sys.stdout.reconfigure(errors="replace")
    except (AttributeError, OSError):
        pass
    if not TARGET_LANGUAGES:
        raise SystemExit(
            "Укажите языки в TARGET_LANGUAGES в начале translations/translate_ts.py."
        )
    if not OLLAMA_MODEL.strip():
        raise SystemExit("Укажите OLLAMA_MODEL в начале translations/translate_ts.py.")
    if BATCH_SIZE < 1:
        raise SystemExit("BATCH_SIZE должен быть больше нуля.")
    if LIMIT < 0:
        raise SystemExit("LIMIT не может быть отрицательным.")
    ensure_model()
    for code in TARGET_LANGUAGES:
        translate_language(code, prepare_catalog())


if __name__ == "__main__":
    main()
