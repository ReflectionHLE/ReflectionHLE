-- MojoSetup; a portable, flexible installation application.
--
-- Please see the file LICENSE.txt in the source's root directory.
--
--  This file written by Ryan C. Gordon.


-- This is a setup file. Lines that start with "--" are comments.
--  This config file is actually just Lua code. Even though you'll only
--  need a small subset of the language, there's a lot of flexibility
--  available to you if you need it.   http://www.lua.org/
--
-- All functionality supplied by the installer is encapsulated in either the
--  "Setup" or "MojoSetup" table, so you can use any other symbol name without
--  namespace clashes, assuming it's not a Lua keyword or a symbol supplied
--  by the standard lua libraries.
--
-- So here's the actual configuration...we used loki_setup's xml schema
--  as a rough guideline.

Setup.Package
{
    vendor = "NY00123",
    id = "reflection-keen",
    description = "Reflection Keen",
    version = "0.18.0",
    --splash = "splash.bmp",
    superuser = false,
    recommended_destinations = { MojoSetup.info.homedir },
    --updateurl = "http://localhost/updates/",
    write_manifest = true,
    support_uninstall = true,

    -- Things named Setup.Something are internal functions we supply.
    --  Generally these return the table you pass to them, but they
    --  may sanitize the values, add defaults, and verify the data.

    -- End User License Agreement(s). You can specify multiple
    --  Setup.Eula sections here.
    --  Also, Note the "translate" call.
    --  This shows up as the first thing the user sees, and must
    --  agree to before anything goes forward. You could put this
    --  in the base Setup.Option and they won't be shown it until
    --  installation is about to start, if you would rather
    --  defer this until necessary and/or show the README first.
    Setup.Eula
    {
        description = "GNU General Public License",
        source = MojoSetup.translate("LICENSE"),
    },

    -- README file(s) to show and install.
    Setup.Readme
    {
        description = "README",
        source = MojoSetup.translate("README.md"),
    },

    -- Specify chunks to install...optional or otherwise.
    Setup.Option
    {
        value = true,
        required = true,
        disabled = false,
        bytes = 6300000,
        description = "Base Install",

        -- Install all contents of data directory
        Setup.File
        {
        },

        -- Install desktop menu items with the base install.
        Setup.DesktopMenuItem
        {
            disabled = false,
            name = "Reflection Keen Dreams",
            genericname = "Reflection Keen Dreams",
            tooltip = "A port of Keen Dreams",
            builtin_icon = false,
            icon = "reflection-kdreams-128x128.png",
            commandline = "%0/reflection-kdreams",
            category = "Game",
            mimetype = { 'application/x-mygame-map', 'application/x-mygame-url' },
        },

        Setup.DesktopMenuItem
        {
            disabled = false,
            name = "Reflection Keen Dreams (Software)",
            genericname = "Reflection Keen Dreams (Software)",
            tooltip = "A port of Keen Dreams (Software)",
            builtin_icon = false,
            icon = "reflection-kdreams-128x128.png",
            commandline = "%0/reflection-kdreams -softlauncher",
            category = "Game",
            mimetype = { 'application/x-mygame-map', 'application/x-mygame-url' },
        },

        Setup.DesktopMenuItem
        {
            disabled = false,
            name = "Reflection Catacomb 3-D",
            genericname = "Reflection Catacomb 3-D",
            tooltip = "A port of Catacomb 3-D",
            builtin_icon = false,
            icon = "reflection-cat3d-128x128.png",
            commandline = "%0/reflection-cat3d",
            category = "Game",
            mimetype = { 'application/x-mygame-map', 'application/x-mygame-url' },
        },

        Setup.DesktopMenuItem
        {
            disabled = false,
            name = "Reflection Catacomb 3-D (Software)",
            genericname = "Reflection Catacomb 3-D (Software)",
            tooltip = "A port of Catacomb 3-D (Software)",
            builtin_icon = false,
            icon = "reflection-cat3d-128x128.png",
            commandline = "%0/reflection-cat3d -softlauncher",
            category = "Game",
            mimetype = { 'application/x-mygame-map', 'application/x-mygame-url' },
        },

        Setup.DesktopMenuItem
        {
            disabled = false,
            name = "Reflection Catacomb Abyss",
            genericname = "Reflection Catacomb Abyss",
            tooltip = "A port of Catacomb Abyss",
            builtin_icon = false,
            icon = "reflection-catabyss-128x128.png",
            commandline = "%0/reflection-catabyss",
            category = "Game",
            mimetype = { 'application/x-mygame-map', 'application/x-mygame-url' },
        },

        Setup.DesktopMenuItem
        {
            disabled = false,
            name = "Reflection Catacomb Abyss (Software)",
            genericname = "Reflection Catacomb Abyss (Software)",
            tooltip = "A port of Catacomb Abyss (Software)",
            builtin_icon = false,
            icon = "reflection-catabyss-128x128.png",
            commandline = "%0/reflection-catabyss -softlauncher",
            category = "Game",
            mimetype = { 'application/x-mygame-map', 'application/x-mygame-url' },
        },

        Setup.DesktopMenuItem
        {
            disabled = false,
            name = "Reflection Catacomb Armageddon",
            genericname = "Reflection Catacomb Armageddon",
            tooltip = "A port of Catacomb Armageddon",
            builtin_icon = false,
            icon = "reflection-catarm-128x128.png",
            commandline = "%0/reflection-catarm",
            category = "Game",
            mimetype = { 'application/x-mygame-map', 'application/x-mygame-url' },
        },

        Setup.DesktopMenuItem
        {
            disabled = false,
            name = "Reflection Catacomb Armageddon (Software)",
            genericname = "Reflection Catacomb Armageddon (Software)",
            tooltip = "A port of Catacomb Armageddon (Software)",
            builtin_icon = false,
            icon = "reflection-catarm-128x128.png",
            commandline = "%0/reflection-catarm -softlauncher",
            category = "Game",
            mimetype = { 'application/x-mygame-map', 'application/x-mygame-url' },
        },

        Setup.DesktopMenuItem
        {
            disabled = false,
            name = "Reflection Catacomb Apocalypse",
            genericname = "Reflection Catacomb Apocalypse",
            tooltip = "A port of Catacomb Apocalypse",
            builtin_icon = false,
            icon = "reflection-catapoc-128x128.png",
            commandline = "%0/reflection-catapoc",
            category = "Game",
            mimetype = { 'application/x-mygame-map', 'application/x-mygame-url' },
        },

        Setup.DesktopMenuItem
        {
            disabled = false,
            name = "Reflection Catacomb Apocalypse (Software)",
            genericname = "Reflection Catacomb Apocalypse (Software)",
            tooltip = "A port of Catacomb Apocalypse (Software)",
            builtin_icon = false,
            icon = "reflection-catapoc-128x128.png",
            commandline = "%0/reflection-catapoc -softlauncher",
            category = "Game",
            mimetype = { 'application/x-mygame-map', 'application/x-mygame-url' },
        },
    },
}

-- end of config.lua ...

