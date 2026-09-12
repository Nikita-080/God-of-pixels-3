#include "planet.h"
#include "planet_p.h"

void Planet::Name()
{
    if (s.name_algorithm == 1)
        name = Name_gop2();
    else if (s.name_algorithm == 2)
        name = Name_readable();
    else
        name = Name_random();
}

QString Planet::Name_random()
{
    QString alfa = "qwertyuiopasdfghjklzxcvbnm1234567890";
    QString s = "";
    int len = RAND(3, 10);
    for (int i = 0; i < len; i++)
        s += alfa[RAND(0, 35)];
    return s;
}

QString Planet::Name_gop2()
{
    QString sogl = "rtpsdfghkljzxcvbnmy";
    QString glas = "euioa";
    QString alfa = sogl + glas;
    QString s = "";
    int len = RAND(3, 8);
    s += alfa[RAND(0, 23)];
    s += alfa[RAND(0, 23)];
    for (int i = 0; i < len - 2; i++)
    {
        if (sogl.contains(s[i + 1]) and sogl.contains(s[i]))
            s += glas[RAND(0, 4)];
        else if (glas.contains(s[i + 1]) and glas.contains(s[i]))
            s += sogl[RAND(0, 18)];
        else
            s += alfa[RAND(0, 23)];
    }
    int num = RAND(1, 1000);
    return s + "-" + QString::number(num);
}

QString Planet::Name_readable()
{
    const QVector<QVector<int>> &word_table = planetNameMatrix();
    QString alfa = "abcdefghijklmnopqrstuvwxyz";
    QString sogl = "rtpsdfghkljzxcvbnmy";
    QString glas = "euioa";
    QString s = "";
    s += alfa[RAND(0, 25)];
    s += char2char(s[0], word_table);
    int len = RAND(1, 6);
    for (int i = 0; i < len; i++)
    {
        if ((sogl.contains(s[i + 1]) and sogl.contains(s[i])) or s[i + 1] == 'x')
        {
            QChar c = sogl[0];
            while (sogl.contains(c))
                c = char2char(s[i + 1], word_table);
            s += c;
        }
        else if (glas.contains(s[i + 1]) and glas.contains(s[i]))
        {
            QChar c = glas[0];
            while (glas.contains(c))
                c = char2char(s[i + 1], word_table);
            s += c;
        }
        else
            s += char2char(s[i + 1], word_table);
    }
    return s[0].toUpper() + s.mid(1, len + 1);
}

QChar Planet::char2char(QChar a, const QVector<QVector<int>> &word_table)
{
    QString alfa = "abcdefghijklmnopqrstuvwxyz";
    const QVector<int> &work_arr = word_table[alfa.indexOf(a)];
    int work_arr_sum = 0;
    for (int i = 0; i < 26; i++)
        work_arr_sum += work_arr[i];
    int v = RAND(1, work_arr_sum);
    int ptr = 0;
    int summa = 0;
    while (summa < v)
    {
        summa += work_arr[ptr];
        ptr++;
    }
    ptr -= 1;
    return alfa[ptr];
}
