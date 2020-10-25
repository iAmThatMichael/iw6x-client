#include <std_include.hpp>
#include "loader/module_loader.hpp"
#include "localized_strings.hpp"
#include "scheduler.hpp"
#include "game/game.hpp"

#include "utils/hook.hpp"
#include "utils/string.hpp"

namespace branding
{
	namespace
	{
		utils::hook::detour ui_get_formatted_build_number_hook;
		
		void dvar_set_string_stub(game::dvar_t* dvar, const char* string)
		{
			game::Dvar_SetString(dvar, utils::string::va("IW6x %s (game %s)", VERSION, string));
		}

		const char* ui_get_formatted_build_number_stub()
		{
			const auto* const build_num = ui_get_formatted_build_number_hook.invoke<const char*>();
			
			return utils::string::va("%s (%s)", VERSION, build_num);
		}
	}
	
	class module final : public module_interface
	{
	public:
		void post_unpack() override
		{
			if (game::environment::is_dedi())
			{
				return;
			}

			if (game::environment::is_mp())
			{
				localized_strings::override("LUA_MENU_MULTIPLAYER_CAPS", "IW6x: MULTIPLAYER\n");
			}

			localized_strings::override("LUA_MENU_LEGAL_COPYRIGHT", "IW6x: Pre-Release by X Labs.\n");

			utils::hook::call(SELECT_VALUE(0x1403BDABA, 0x140414424), dvar_set_string_stub);
			ui_get_formatted_build_number_hook.create(SELECT_VALUE(0x140415FD0, 0x1404D7C00), ui_get_formatted_build_number_stub);

			scheduler::loop([]() {
				const auto x = 24;
				const auto y = 24;
				const auto scale = 0.5f;
				float color[4] = {0.666f, 0.666f, 0.666f, 0.666f};
				const auto *text = "IW6x: Pre-Release\n" VERSION;

				auto *font = game::R_RegisterFont("fonts/normalfont");
				if (!font)
					return;

				game::R_AddCmdDrawText(text, 0x7FFFFFFF, font, x, y + static_cast<float>(font->pixelHeight) * scale,
									   scale, scale, 0.0, color, 0);
			},
							scheduler::pipeline::renderer);
		}
	};
} // namespace branding

REGISTER_MODULE(branding::module)
