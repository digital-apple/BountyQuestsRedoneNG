set_xmakever("2.8.2")

includes("libraries/commonlibsse-ng")

set_project("Bounty Quests Redone - NG")
set_version("1.0.6")
set_license("MIT")

set_languages("c++23")
set_warnings("allextra")

set_policy("package.requires_lock", true)

add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

set_config("skyrim_vr", false)

add_requires("jsoncons")

target("Bounty Quests Redone - NG")
	add_deps("commonlibsse-ng")
	
    add_packages("jsoncons")

	add_rules("commonlibsse-ng.plugin", {
        name = "Bounty Quests Redone - NG",
        author = "digital-apple"
    })

    add_files("source/**.cpp")
    add_headerfiles("include/**.h")
    add_includedirs("include", { public = true })
    set_pcxxheader("include/PCH.h")

    after_build(function(target)
        local copy = function(env, ext)
            for _, env in pairs(env:split(";")) do
                if os.exists(env) then
                    local plugins = path.join(env, ext, "SKSE/Plugins")
                    os.mkdir(plugins)
                    os.trycp(target:targetfile(), plugins)
                    os.trycp(target:symbolfile(), plugins)
                end
            end
        end
        if os.getenv("XSE_TES5_MODS_PATH") then
            copy(os.getenv("XSE_TES5_MODS_PATH"), target:name())
        elseif os.getenv("XSE_TES5_GAME_PATH") then
            copy(os.getenv("XSE_TES5_GAME_PATH"), "Data")
        end
    end)
