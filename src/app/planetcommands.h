#ifndef PLANETCOMMANDS_H
#define PLANETCOMMANDS_H

#include <QUndoCommand>
#include <functional>
#include "planet.h"
#include "planetsettings.h"

class SettingsUndoCommand : public QUndoCommand
{
public:
    using Apply = std::function<void(const PlanetSettings &, bool)>;

    SettingsUndoCommand(const PlanetSettings &before, const PlanetSettings &after, bool appearanceOnly, Apply apply);

    void undo() override;
    void redo() override;

private:
    PlanetSettings before;
    PlanetSettings after;
    bool appearanceOnly;
    Apply apply;
    bool virgin;
};

class PlanetUndoCommand : public QUndoCommand
{
public:
    using Apply = std::function<void(const Planet &)>;

    PlanetUndoCommand(const Planet &before, const Planet &after, Apply apply);

    void undo() override;
    void redo() override;

private:
    Planet before;
    Planet after;
    Apply apply;
    bool virgin;
};

#endif
