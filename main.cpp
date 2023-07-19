#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[]){
    //строки ниже портят программу, и я не помню, зачем их писал. надо разобраться
    //QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //QApplication::setStyle(QStyleFactory::create("Fusion"));
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
/* TODO LOG
 * [ok] [bug] нулевая прозрачность атмосферы - пропадает атмосфера
 * [ok] максимальная прозрачность атмосферы - пропадает атмосфера
 * [ok] [bug] максимальное освещение - теневой круг вокруг ТМО
 * запоминать путь сохранения
 * [ok] цвет из кнопки в колор-диалог
 * [del] [wrong] панель кнопок обвести серой рамкой
 * [ok] неоновые цвета интерфейса
 * [ok] подсветка при наведении курсора (затемнение при нажатии)
 * [ok] карта влажности
 * [ok] карта температуры
 * [del] [not_actual] карта биомов
 * [ok] разные цвета растений
 * [ok] интенсивность пикселя растений
 * [?] подробный отчет
 * распределение начальных букв имени
 * [?] имя в зависимости от цвета
 * [?] убрать масштабирование и ручное распределение высот
 * [del] [not_actual] распределение людей от влажности и температуры
 * [del] интенсивность пикселя людей
 * [del] [not_actual] ручное программирование вкладок
 * [?] корректировка высот в соответствии со структурой
 * [not_actual] полюс = дуга - экватор
 * [ok][?] случайность 0.05 (0,1...0,01...1,0)
 * [ok][?] блуждающая планета
 * двойная звездная система
 * [del] [not_actual] исправить расстояние до экватора с учетом uv
 * [del] кольца из частиц
 * [ok] сейсмическая активность (уровень)
 * [del] [wrong] трещины с лавой
 * [ok] меню - о программе
 * [del] [not_actual] меню - помощь
 * [ok] зависимость жизни не от ползунка
 * [ok] убрать прозрачность растений (поменять интерпретацию)
 * [ok] убрать людей
 * полосатая атмосфера (как у Юпитера)
 * [ok] зависимость климата от оси вращения
 * [ok] зависимость расстояния до звезды от температуры
 * [ok] понизить минимальную температуру
 * [?] обработка ошибок autogod
 * [ok] цвет меню
 * [ok] цвет вкладок
 * [ok] [bug] конфликт тени и атмосферы (возможно удалить радиус атмосферы)
 * [ok] [bug] количество растений в description в несколько десятков
 * [ok] дизайн hover кнопки меню
 * [ok] добавить какой-то элемент под кнопками выбора картинки
 * [ok] [bug] белые вкладки
 * [del] [wrong] возведение теневого коэффициента в степень
 * [ok] [bug] случайная структура приводит к темным пятнам
 * [ok] [bug] кольца выскакивают за максимальный индекс цвета
 * [ok] [bug] не работает детализация облаков
 * [ok] fault formation
 * [ok] размытие карты биомов
 * [ok] градиентное окрашивание (вкл/выкл) (вода, суша, ледники, растения, кольца)
 * [del] [wrong] освещение колец должно зависеть от точки освещения?
 * [ok] разделить градиенты воды и суши
 * [del] [wrong] масштабирование структуры по Земле (изменение умолчаний)
 * [ok] шум пикселей
 * [del] [wrong] переменное количество точек окрашивания
 * [ok] пересмотреть изменения при изменении планеты
 * [del] [wrong] заменить "жизнь" на другую характеристику в описании
 * прегенерация 4 UV карт
 * [ok] [bug] максимальный уровень воды переходит границу прогрессбара
 * [bug] максимальный уровень льда недоходит до границы прогрессбара <- лед не покрывает периметр мира
 * добавить новые уровни в разделе description
 * тектоническая симуляция
*/

/*
 * статистика
 * 24.01.2023
 * .h - 385 строк
 * .cpp - 2105 строк
 * 19.07.2023
 * .h - 419 строк
 * .cpp - 2228 строк
*/
