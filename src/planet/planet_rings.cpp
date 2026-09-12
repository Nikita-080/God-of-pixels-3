#include "planet.h"
#include <QtMath>

void Planet::PrepareRings()
{
    rnd.seed(static_cast<quint32>(seed));
    ring_inner = 1.18 + 0.08 * s.R_internal_ring;
    ring_outer = 1.55 + 0.12 * s.R_external_ring;
    if (ring_outer <= ring_inner + 0.05)
        ring_outer = ring_inner + 0.25;
    ring_colors.clear();
    ring_colors_dark.clear();
    ring_bands.clear();
    ring_rocks.clear();
    if (!s.is_ring)
        return;

    const int filled = RAND(4, 8);
    const int intensity = qBound(0, s.ring_intensity, 10);
    const int empty = (s.ring_material == 1) ? 0 : (10 - intensity);
    const int total = qMax(1, filled + empty);
    QVector<int> kinds(total, 1);
    for (int i = 0; i < empty; ++i)
        kinds[i] = 0;
    for (int i = total - 1; i > 0; --i)
        qSwap(kinds[i], kinds[RAND(0, i)]);

    const int colorSpread = 30;
    const float span = float(ring_outer - ring_inner);
    const float step = span / float(total);
    for (int i = 0; i < total; ++i)
    {
        RingBand band;
        band.inner = float(ring_inner) + step * i;
        band.outer = band.inner + step;
        band.empty = kinds[i] == 0;
        if (band.empty)
        {
            band.color = Qt::transparent;
        }
        else
        {
            band.color = DispersionColor(s.ring_color, colorSpread);
            ring_colors.append(band.color);
            ring_colors_dark.append(LowerColor(band.color, 0.5));
        }
        ring_bands.append(band);
    }

    if (s.ring_material != 1)
        return;

    const int count = 15 + intensity * 18;
    const double inner2 = ring_inner * ring_inner;
    const double outer2 = ring_outer * ring_outer;
    for (int i = 0; i < count; ++i)
    {
        const double u = rnd.generateDouble();
        const double r = sqrt(inner2 + u * (outer2 - inner2));
        const double th = rnd.generateDouble() * 2.0 * M_PI;
        RingRock rock;
        rock.x = float(r * cos(th));
        rock.y = 0.0f;
        rock.z = float(r * sin(th));
        rock.radius = float(0.04 + rnd.generateDouble() * 0.08);
        if (!ring_colors.isEmpty())
            rock.color = ring_colors[RAND(0, ring_colors.size() - 1)];
        else
            rock.color = DispersionColor(s.ring_color, colorSpread);
        ring_rocks.append(rock);
    }
}
