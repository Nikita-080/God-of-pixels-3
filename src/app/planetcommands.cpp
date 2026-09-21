#include "planetcommands.h"

SettingsUndoCommand::SettingsUndoCommand(const PlanetSettings &beforeSettings,
                                         const PlanetSettings &afterSettings,
                                         bool appearance,
                                         Apply applyFn)
    : before(beforeSettings)
    , after(afterSettings)
    , appearanceOnly(appearance)
    , apply(std::move(applyFn))
    , virgin(true)
{
}

void SettingsUndoCommand::undo()
{
    if (apply)
        apply(before, appearanceOnly);
}

void SettingsUndoCommand::redo()
{
    if (virgin)
    {
        virgin = false;
        return;
    }
    if (apply)
        apply(after, appearanceOnly);
}

PlanetUndoCommand::PlanetUndoCommand(const Planet &beforePlanet, const Planet &afterPlanet, Apply applyFn)
    : before(beforePlanet)
    , after(afterPlanet)
    , apply(std::move(applyFn))
    , virgin(true)
{
}

void PlanetUndoCommand::undo()
{
    if (apply)
        apply(before);
}

void PlanetUndoCommand::redo()
{
    if (virgin)
    {
        virgin = false;
        return;
    }
    if (apply)
        apply(after);
}
