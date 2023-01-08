#include "Birb2D.hpp"
#include <iostream>
#include <iterator>
#include <string>

using namespace Birb;

/* Function declarations */
static void start(Game& game);
static void input(Game& game);
static void update(Game& game);
static void render(Game& game);
static void post_render();
static void cleanup();

/* Sounds */
Audio::SoundFile laptop_typing;
Audio::SoundFile mechanical_typing;
Audio::SoundFile power_button;
Audio::MusicFile salt_falling_sound;

Audio::SoundFile wheat_harvest_sound;

Audio::SoundFile glass_hit;

/* Textures */

/* No minigames finished yet */
Texture room_default_texture;
Texture wheat_flour_empty;
Texture water_empty;
Texture salt_empty;

/* Wheat collected */
Texture table_wheat_normal_tex;
Texture table_wheat_water_tex;
Texture table_wheat_salt_tex;

/* Water collected */
Texture table_water_normal_tex;
Texture table_water_wheat_tex;
Texture table_water_salt_tex;

/* Salt collected */
Texture table_salt_normal_tex;
Texture table_salt_water_tex;
Texture table_salt_wheat_tex;

/* Wheat and water collected */
Texture table_wheat_water_normal_tex;
Texture table_wheat_water_salt_tex;

/* Wheat and salt collected */
Texture table_wheat_salt_normal_tex;
Texture table_wheat_salt_water_tex;

/* Water and salt collected */
Texture table_water_salt_normal_tex;
Texture table_water_salt_wheat_tex;

/* Wheat, water and salt collected */
Texture everything_collected_normal;
Texture everything_collected_hover;

/* Wheat whacking game */
Texture whack_a_wheat_empty;
Texture whack_a_wheat[4];
Texture whack_a_wheat_return_normal;
Texture whack_a_wheat_return_hover;

/* Water collecting game */
/* Format: [bowl position left to right] [button highlight] [fill amount] */
Texture water_bowl[3][3][3];
Texture water_bowl_return_normal[3];
Texture water_bowl_return_return[3];
Texture water_bowl_return_left[3];
Texture water_bowl_return_right[3];

/* Salt collecting game */
Texture salt_normal;
Texture salt_start_button_hover;
Texture salt_display_on[3];
Texture salt_fall_steps[10];
Texture salt_fall_done;
Texture salt_return_normal;
Texture salt_return_hover;

/** Hitboxes **/
/* Table */
Rect wheat_flour(360, 135, 140, 190);
Rect water(725, 215, 155, 105);
Rect salt(915, 290, 70, 85);
Rect spawn_bread_hitbox(437, 290, 334, 85);

/* Whack-A-Wheat */
Rect wheat_hitbox[4] = {
	Rect(524, 197, 53, 121),
	Rect(715, 206, 47, 125),
	Rect(476, 248, 49, 141),
	Rect(679, 266, 54, 143)
};
Rect wheat_return_hitbox(0, 466, 351, 90);

/* Water collecting */
Rect left_arrow_hitbox(166, 548, 103, 30);
Rect right_arrow_hitbox(129, 598, 107, 37);
Rect water_return_hitbox(9, 482, 339, 56);

/* Salt collection */
Rect saltynator_button_hitbox(631, 519, 45, 27);
Rect salt_return_hitbox = water_return_hitbox;


enum Item
{
	NONE 		= 0,
	WHEAT_FLOUR = 1,
	WATER 		= 2,
	SALT 		= 3
};

Item last_item 		= NONE;
Item current_item 	= NONE;

enum ArrowHover
{
	NEITHER 	= 0,
	LEFT 		= 1,
	RIGHT 		= 2,
	RETURN 		= 3
};

ArrowHover current_arrow 	= NEITHER;
ArrowHover last_arrow 		= NEITHER;

enum GameScene
{
	TABLE_ROOM,
	WHACK_A_WHEAT,
	WATER_COLLECTING,
	SALT_COLLECTION
};

GameScene last_scene 		= TABLE_ROOM;
GameScene current_scene 	= TABLE_ROOM;

Scene room;
Entity room_entity;
Font* free_font_bold;

/* Whack-A-Wheat variables */
Timer wheat_timer;
int current_wheat = -1;
int collected_wheat_count = 0;
bool wheat_picked = false;
bool wheat_whacking_done = false;
Entity wheat_collected_text;
bool hovering_over_wheat_return_text 		= false;
bool hovering_over_wheat_return_text_last 	= false;

struct CrossHair
{
	Rect horizontal = Rect(0, 16, 32, 4, Colors::Nord::SnowStorm::nord6);
	Rect vertical 	= Rect(16, 0, 4, 32, Colors::Nord::SnowStorm::nord6);
	bool active;

	CrossHair()
	{
		active = false;
	}

	void Activate()
	{
		active = true;
	}

	void Deactivate()
	{
		active = false;
	}

	void SetPos(Vector2Int pos)
	{
		horizontal.x = pos.x - 16;
		horizontal.y = pos.y - 2;

		vertical.x = pos.x - 2;
		vertical.y = pos.y - 16;
	}

	void Render()
	{
		if (active)
		{
			Render::DrawRect(horizontal);
			Render::DrawRect(vertical);
		}
	}
};
CrossHair crosshair;


/* Water collecting variables */
bool bowl_position_updated 		= false;
int current_bowl_position 		= 0;
int current_bowl_fill_amount 	= 0;
int falling_waters_collected 	= 0;

Entity falling_water_text;
int falling_water_position 		= -1;
bool water_is_falling 			= false;
bool water_return_scene_active 	= false;
bool water_collecting_done 		= false;

/* Salt collecting variables */
bool start_button_hover 			= false;
bool start_button_hover_last 		= false;
bool machine_started 				= false;
bool salt_falling 					= false;
int chat_step 						= 0;
int salt_falling_step 				= 0;
bool salt_collecting_done 			= false;
bool hovering_over_salt_return 		= false;
bool hovering_over_salt_return_last = false;
Timer salt_timer;

/* Spawn bread variables */
bool hovering_spawn_text 		= false;
bool hovering_spawn_text_last 	= false;
bool spawn_the_bread 			= false;

Texture breads[18];
int bread_count 	= 18;
int current_bread 	= -1;
int last_bread 		= -1;


Input::MouseDrag drag_controller;
Rect drag_rect;


int main(int argc, char** argv)
{
	Game::WindowOpts window_options;
	window_options.title 				= "Let's get this bread";
	window_options.window_dimensions 	= { 1280, 720 };
	window_options.refresh_rate 		= 60;
	window_options.resizable 			= false;

	Game game_loop(window_options, start, input, update, render);

	/* Optional extra functions */
	game_loop.post_render = post_render;
	game_loop.cleanup = cleanup;

	/* Start the game loop */
	game_loop.Start();

	return 0;
}

/* start() is called before the game loop starts.
 * Useful for doing stuff that will only run once before
 * the game starts */
void start(Game& game)
{
	/* Show the splash screen */
	Splash splash(*game.window);
	splash.loading_text = "Loading... This is gonna take a single (1) moment";
	splash.Run();

	/* Load resources */
	laptop_typing 		= Audio::SoundFile("sounds/laptop_typing.wav");
	mechanical_typing 	= Audio::SoundFile("sounds/mechanical_typing.wav");
	power_button 		= Audio::SoundFile("sounds/power_button.wav");
	salt_falling_sound	= Audio::MusicFile("sounds/salt_falling.wav");

	wheat_harvest_sound = Audio::SoundFile("sounds/wheat_harvest.wav");

	glass_hit 			= Audio::SoundFile("sounds/glass_hit.wav");

	free_font_bold = new Font("fonts/FreeMonoBold.ttf", 28);
	room_default_texture.LoadTexture("textures/table_default.jpg");
	wheat_flour_empty.LoadTexture("textures/wheat_flour.jpg");
	water_empty.LoadTexture("textures/water.jpg");
	salt_empty.LoadTexture("textures/salt.jpg");

	table_wheat_normal_tex.LoadTexture("textures/table_wheat/normal.jpg");
	table_wheat_water_tex.LoadTexture("textures/table_wheat/water.jpg");
	table_wheat_salt_tex.LoadTexture("textures/table_wheat/salt.jpg");

	table_water_normal_tex.LoadTexture("textures/table_water/normal.jpg");
	table_water_wheat_tex.LoadTexture("textures/table_water/wheat.jpg");
	table_water_salt_tex.LoadTexture("textures/table_water/salt.jpg");

	table_wheat_water_normal_tex.LoadTexture("textures/table_wheat_water/normal.jpg");
	table_wheat_water_salt_tex.LoadTexture("textures/table_wheat_water/salt.jpg");

	table_water_salt_normal_tex.LoadTexture("textures/table_water_salt/normal.jpg");
	table_water_salt_wheat_tex.LoadTexture("textures/table_water_salt/wheat.jpg");

	table_salt_normal_tex.LoadTexture("textures/table_salt/normal.jpg");
	table_salt_water_tex.LoadTexture("textures/table_salt/water.jpg");
	table_salt_wheat_tex.LoadTexture("textures/table_salt/wheat.jpg");

	table_wheat_salt_normal_tex.LoadTexture("textures/table_wheat_salt/normal.jpg");
	table_wheat_salt_water_tex.LoadTexture("textures/table_wheat_salt/water.jpg");


	/* Wheat whacking game */
	wheat_collected_text = Entity("Wheat collected",
			Vector2Int(32, 32),
			EntityComponent::Text("Wheat collected: 0 / 10", free_font_bold, &Colors::Nord::PolarNight::nord1),
			2);
	wheat_collected_text.active = false;
	room.AddObject(&wheat_collected_text);

	whack_a_wheat_empty.LoadTexture("textures/whack-a-wheat_empty.jpg");
	whack_a_wheat_return_normal.LoadTexture("textures/whack-a-wheat_return_to_table_normal.jpg");
	whack_a_wheat_return_hover.LoadTexture("textures/whack-a-wheat_return_to_table_hover.jpg");
	for (int i = 1; i < 5; ++i)
		whack_a_wheat[i - 1].LoadTexture("textures/whack-a-wheat_" + std::to_string(i) + ".jpg");

	/* Water collecting game */
	for (int i = 1; i < 4; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			water_bowl[i - 1][0][j].LoadTexture("textures/water_collecting/" + std::to_string(i) + "/normal_" + std::to_string(j) + ".jpg");
			water_bowl[i - 1][1][j].LoadTexture("textures/water_collecting/" + std::to_string(i) + "/left_" + std::to_string(j) + ".jpg");
			water_bowl[i - 1][2][j].LoadTexture("textures/water_collecting/" + std::to_string(i) + "/right_" + std::to_string(j) + ".jpg");
		}

		/* Return texts */
		water_bowl_return_left[i - 1].LoadTexture("textures/water_collecting/return_to_table/left_" + std::to_string(i) + ".jpg");
		water_bowl_return_normal[i - 1].LoadTexture("textures/water_collecting/return_to_table/normal_" + std::to_string(i) + ".jpg");
		water_bowl_return_return[i - 1].LoadTexture("textures/water_collecting/return_to_table/return_" + std::to_string(i) + ".jpg");
		water_bowl_return_right[i - 1].LoadTexture("textures/water_collecting/return_to_table/right_" + std::to_string(i) + ".jpg");
	}
	falling_water_text = Entity("Falling water text", Vector2Int(0, -500), EntityComponent::Text("Falling\n water", free_font_bold, &Colors::Nord::PolarNight::nord3), 2);
	falling_water_text.active = false;
	room.AddObject(&falling_water_text);


	/* Salt collecting game */
	salt_normal.LoadTexture("textures/salt_collecting/normal.jpg");
	salt_start_button_hover.LoadTexture("textures/salt_collecting/start_button_hover.jpg");

	for (int i = 0; i < 3; ++i)
		salt_display_on[i].LoadTexture("textures/salt_collecting/display_on_" + std::to_string(i) + ".jpg");

	for (int i = 0; i < 10; ++i)
		salt_fall_steps[i].LoadTexture("textures/salt_collecting/salt_fall_step_" + std::to_string(i) + ".jpg");

	salt_fall_done.LoadTexture("textures/salt_collecting/salt_collecting_done.jpg");
	salt_return_normal.LoadTexture("textures/salt_collecting/return.jpg");
	salt_return_hover.LoadTexture("textures/salt_collecting/return_hover.jpg");

	/* Final game scene */
	everything_collected_normal.LoadTexture("textures/table_wheat_water_salt/spawn_bread_normal.jpg");
	everything_collected_hover.LoadTexture("textures/table_wheat_water_salt/spawn_bread_hover.jpg");
	current_bread = Global::random.RandomInt(0, bread_count - 1);

	for (int i = 0; i < bread_count; ++i)
	{
		breads[i].LoadTexture("textures/breads/" + std::to_string(i) + ".jpg");
	}


	room_entity = Entity("Room default", Rect(0, 0, game.window->dimensions.x, game.window->dimensions.y), room_default_texture, 0);
	room.AddObject(&room_entity);

	/* Display the main menu */
	MainMenuSettings main_menu_settings;
	main_menu_settings.title.text = "Let's get this bread";
	main_menu_settings.credits_menu.credits_text = "Made by ToasterBirb\nLudum Dare 52";
	MainMenu main_menu(game, main_menu_settings);
	main_menu.Launch();
}

/* input() is called at the beginning of the frame
 * before update(). Behind the curtains it does input
 * polling etc. and then passes the SDL_Event into
 * this function */
void input(Game& game)
{
	drag_controller.Poll(game.window->event);

	//if (drag_controller.isDragging())
	//{
	//	drag_rect.x = drag_controller.startPos().x;
	//	drag_rect.y = drag_controller.startPos().y;
	//	drag_rect.w = drag_controller.endPos().x - drag_rect.x;
	//	drag_rect.h = drag_controller.endPos().y - drag_rect.y;
	//	std::cout << "Drag area: " << drag_rect << std::endl;
	//}

	/* Scene input */
	switch (current_scene)
	{
		case (TABLE_ROOM):
		{
			/* Set the background sprite according to cursor position */
			if (game.window->CursorInRect(wheat_flour) && !wheat_whacking_done)
			{
				current_item = WHEAT_FLOUR;

				if (game.window->isMouseDown() && !wheat_whacking_done)
				{
					current_scene = WHACK_A_WHEAT;
					wheat_timer.Start();
				}
			}
			else if (game.window->CursorInRect(water) && !water_collecting_done)
			{
				current_item = WATER;

				if (game.window->isMouseDown())
					current_scene = WATER_COLLECTING;
			}
			else if (game.window->CursorInRect(salt) && !salt_collecting_done)
			{
				current_item = SALT;

				if (game.window->isMouseDown())
					current_scene = SALT_COLLECTION;
			}
			else if (!spawn_the_bread && wheat_whacking_done && water_collecting_done && salt_collecting_done && game.window->CursorInRect(spawn_bread_hitbox))
			{
				hovering_spawn_text = true;

				/* Check if the spawn bread text is clicked */
				if (game.window->isMouseDown())
				{
					spawn_the_bread = true;
				}
			}
			else if (wheat_whacking_done && water_collecting_done && salt_collecting_done && !spawn_the_bread)
			{
				hovering_spawn_text = false;
			}
			else if (spawn_the_bread && (game.window->isMouseDown() || game.window->isKeyDown()))
			{
				current_bread = Global::random.RandomInt(0, bread_count - 1);
			}
			else
			{
				current_item = NONE;
			}
			break;
		}

		case (WHACK_A_WHEAT):
		{
			/* Check if the player clicks on wheat hitboxes */
			if (!wheat_whacking_done && game.window->isMouseDown())
			{
				for (int i = 0; i < 4; ++i)
				{
					if (game.window->CursorInRect(wheat_hitbox[i]) && current_wheat == i)
					{
						wheat_picked = true;
						wheat_harvest_sound.play();
						crosshair.SetPos(game.window->CursorPosition());
						crosshair.Activate();
					}
				}
			}
			else if (wheat_whacking_done)
			{
				if (game.window->CursorInRect(wheat_return_hitbox))
				{
					hovering_over_wheat_return_text = true;

					/* If the player clicks, go to the main table */
					if (game.window->isMouseDown())
					{
						current_scene = TABLE_ROOM;
						wheat_collected_text.active = false;
					}
				}
				else
				{
					hovering_over_wheat_return_text = false;
				}
			}

			break;
		}

		case (WATER_COLLECTING):
		{
			/* Highlight the arrows if hovering over them */
			if (game.window->CursorInRect(left_arrow_hitbox))
			{
				current_arrow = LEFT;

				if (game.window->isMouseDown() && current_bowl_position > 0)
				{
					current_bowl_position--;
					bowl_position_updated = true;
				}
			}
			else if (game.window->CursorInRect(right_arrow_hitbox))
			{
				current_arrow = RIGHT;

				if (game.window->isMouseDown() && current_bowl_position < 2)
				{
					current_bowl_position++;
					bowl_position_updated = true;
				}
			}
			else if (game.window->CursorInRect(water_return_hitbox))
			{
				current_arrow = RETURN;
				bowl_position_updated = true;

				if (game.window->isMouseDown())
				{
					current_scene = TABLE_ROOM;
				}
			}
			else
			{
				current_arrow = NEITHER;
			}

			break;
		}

		case (SALT_COLLECTION):
		{
			if (game.window->CursorInRect(saltynator_button_hitbox))
			{
				start_button_hover = true;

				if (game.window->isMouseDown() && !machine_started)
				{
					machine_started = true;
					power_button.play();
					salt_timer.Start();
				}
			}
			else
			{
				start_button_hover = false;
			}

			/* Return button */
			if (salt_collecting_done)
			{
				if (game.window->CursorInRect(salt_return_hitbox))
				{
					hovering_over_salt_return = true;

					if (game.window->isMouseDown())
						current_scene = TABLE_ROOM;
				}
				else
				{
					hovering_over_salt_return = false;
				}
			}
			break;
		}
	}
}

/* update() is called after input has been handled and
 * before the frame gets rendered. Its useful for any game
 * logic that needs to be updated before rendering */
void update(Game& game)
{
	/* Handle scene switching */
	if (current_scene != last_scene)
	{
		switch (current_scene)
		{
			case (TABLE_ROOM):
			{
				current_item = NONE;
				room_entity.sprite = room_default_texture;

				if (wheat_whacking_done && !water_collecting_done)
					room_entity.sprite = table_wheat_normal_tex;
				else if (!wheat_whacking_done && water_collecting_done)
					room_entity.sprite = table_water_normal_tex;
				else if (wheat_whacking_done && water_collecting_done)
					room_entity.sprite = table_wheat_water_tex;
				break;
			}

			case (WHACK_A_WHEAT):
			{
				wheat_collected_text.active = true;
				room_entity.sprite = whack_a_wheat_empty;
				break;
			}

			case (WATER_COLLECTING):
			{
				room_entity.sprite = water_bowl[0][0][0];
				break;
			}

			case (SALT_COLLECTION):
			{
				room_entity.sprite = salt_normal;
				break;
			}
		}

		last_scene = current_scene;
	}

	/* Handle game end spawn text highlight */
	if (!spawn_the_bread)
	{
		if (hovering_spawn_text != hovering_spawn_text_last)
		{
			if (hovering_spawn_text)
				room_entity.sprite = everything_collected_hover;
			else
				room_entity.sprite = everything_collected_normal;

			hovering_spawn_text_last = hovering_spawn_text;
		}
	}
	else
	{
		if (current_bread != last_bread)
		{
			room_entity.sprite = breads[current_bread];
			current_bread = last_bread;
		}
	}

	switch (current_scene)
	{
		case (TABLE_ROOM):
		{
			/* Change the sprite if the current item has been changed */
			if (current_item != last_item)
			{
				/* No minigames have been played yet */
				if (!wheat_whacking_done && !water_collecting_done && !salt_collecting_done)
				{
					switch (current_item)
					{
						case (WHEAT_FLOUR):
						{
							room_entity.sprite = wheat_flour_empty;
							break;
						}

						case (WATER):
						{
							room_entity.sprite = water_empty;
							break;
						}

						case (SALT):
						{
							room_entity.sprite = salt_empty;
							break;
						}

						case (NONE):
						{
							room_entity.sprite = room_default_texture;
							break;
						}
					}
				}
				else if (wheat_whacking_done && !water_collecting_done && !salt_collecting_done) /* Wheat has been collected and water, salt not */
				{
					switch (current_item)
					{
						case (WATER):
						{
							room_entity.sprite = table_wheat_water_tex;
							break;
						}

						case (SALT):
						{
							room_entity.sprite = table_wheat_salt_tex;
							break;
						}

						default:
						{
							room_entity.sprite = table_wheat_normal_tex;
							break;
						}
					}
				}
				else if (wheat_whacking_done && !water_collecting_done && salt_collecting_done) /* Wheat and salt have been collected but not water */
				{
					switch (current_item)
					{
						case (WATER):
						{
							room_entity.sprite = table_wheat_salt_water_tex;
							break;
						}

						default:
						{
							room_entity.sprite = table_wheat_salt_normal_tex;
							break;
						}
					}
				}
				else if (water_collecting_done && !wheat_whacking_done && !salt_collecting_done) /* Water has been collected but wheat, salt not */
				{
					switch (current_item)
					{
						case (WHEAT_FLOUR):
						{
							room_entity.sprite = table_water_wheat_tex;
							break;
						}

						case (SALT):
						{
							room_entity.sprite = table_water_salt_tex;
							break;
						}

						default:
						{
							room_entity.sprite = table_water_normal_tex;
							break;
						}
					}
				}
				else if (!water_collecting_done && !wheat_whacking_done && salt_collecting_done) /* Salt collected, but wheat and water not */
				{
					switch (current_item)
					{
						case (WHEAT_FLOUR):
						{
							room_entity.sprite = table_salt_wheat_tex;
							break;
						}

						case (WATER):
						{
							room_entity.sprite = table_salt_water_tex;
							break;
						}

						default:
						{
							room_entity.sprite = table_salt_normal_tex;
							break;
						}
					}
				}
				else if (water_collecting_done && !wheat_whacking_done && salt_collecting_done) /* Water and salt have been collected but not wheat */
				{
					switch (current_item)
					{
						case (WHEAT_FLOUR):
						{
							room_entity.sprite = table_water_salt_wheat_tex;
							break;
						}

						default:
						{
							room_entity.sprite = table_water_salt_normal_tex;
							break;
						}
					}
				}
				else if (water_collecting_done && wheat_whacking_done && !salt_collecting_done) /* Wheat and water has been collected */
				{
					switch (current_item)
					{
						case (SALT):
						{
							room_entity.sprite = table_wheat_water_salt_tex;
							break;
						}

						default:
						{
							room_entity.sprite = table_wheat_water_normal_tex;
							break;
						}
					}
				}
				else if (water_collecting_done && wheat_whacking_done && salt_collecting_done) /* Wheat, water and salt collecting done */
				{
					room_entity.sprite = everything_collected_normal;
				}

				last_item = current_item;
			}
			break;
		}

		case (WHACK_A_WHEAT):
		{
			/* Change the frame randomly and return to the empty frame if the player
			 * successfully clicks on wheat */
			if (wheat_picked)
			{
				room_entity.sprite = whack_a_wheat_empty;
				wheat_picked 	= false;
				current_wheat 	= -1;
				collected_wheat_count++;
				wheat_collected_text.SetText("Wheat collected: " + std::to_string(collected_wheat_count) + " / 10");
				wheat_timer.Start();

				if (collected_wheat_count >= 10)
				{
					wheat_whacking_done = true;
					crosshair.Deactivate();
					room_entity.sprite = whack_a_wheat_return_normal;
				}
				break;
			}

			if (!wheat_whacking_done && wheat_timer.ElapsedSeconds() > Global::random.RandomFloat(0.5f, 2.0f))
			{
				/* Change to a random wheat sprite */
				int index = 0;
				do {
					index = Global::random.RandomInt(0, 3);
				} while (index == current_wheat);
				room_entity.sprite = whack_a_wheat[index];
				current_wheat = index;

				crosshair.Deactivate();

				/* Restart the wheat timer */
				wheat_timer.Start();
			}
			else if (wheat_whacking_done)
			{
				if (hovering_over_wheat_return_text != hovering_over_wheat_return_text_last)
				{
					if (hovering_over_wheat_return_text)
						room_entity.sprite = whack_a_wheat_return_hover;
					else
						room_entity.sprite = whack_a_wheat_return_normal;

					hovering_over_wheat_return_text_last = !hovering_over_wheat_return_text_last;
				}
			}


			break;
		}

		case (WATER_COLLECTING):
		{
			/* If there's no water falling, move the text to the correct position and start
			 * decreasing its y-value */
			if (current_bowl_fill_amount < 2)
			{
				if (!water_is_falling)
				{
					falling_water_text.active 	= true;
					water_is_falling 			= true;

					/* Pick a random bowl position to fall to */
					falling_water_text.rect.y = -50;
					int previous_value = falling_water_position;
					do
					{
						falling_water_position = Global::random.RandomInt(0, 2);
					} while (falling_water_position == previous_value);

					switch (falling_water_position)
					{
						case (0):
						{
							falling_water_text.rect.x = 310;
							break;
						}

						case (1):
						{
							falling_water_text.rect.x = 540;
							break;
						}

						case (2):
						{
							falling_water_text.rect.x = 795;
							break;
						}
					};
				}
				else
				{
					falling_water_text.rect.y += game.time_step()->deltaTime * 300;

					for (int i = 0; i < 3; ++i)
					{
						if (current_bowl_position == i && falling_water_position == i && falling_water_text.rect.y > 640 - ((3 - i) * 40))
						{
							water_is_falling = false;
							falling_water_text.active = false;

							falling_waters_collected++;
							bowl_position_updated = true;

							glass_hit.play();

							if (falling_waters_collected % 2 == 0)
								current_bowl_fill_amount++;
						}
					}

					if (falling_water_text.rect.y > 800)
					{
						water_is_falling = false;
					}
				}
			}
			else if (!water_return_scene_active)
			{
				water_return_scene_active 	= true;
				bowl_position_updated 		= true;
				water_collecting_done 		= true;
			}

			if (current_arrow != last_arrow || bowl_position_updated)
			{
				if (water_return_scene_active)
				{
					switch (current_arrow)
					{
						case (NEITHER):
						{
							room_entity.sprite = water_bowl_return_normal[current_bowl_position];
							break;
						}

						case (RETURN):
						{
							room_entity.sprite = water_bowl_return_return[current_bowl_position];
							break;
						}

						case (LEFT):
						{
							room_entity.sprite = water_bowl_return_left[current_bowl_position];
							break;
						}

						case (RIGHT):
						{
							room_entity.sprite = water_bowl_return_right[current_bowl_position];
							break;
						}
					}
				}
				else
				{
					switch (current_arrow)
					{
						case (NEITHER):
						{
							room_entity.sprite = water_bowl[current_bowl_position][0][current_bowl_fill_amount];
							break;
						}

						case (RETURN):
						{
							room_entity.sprite = water_bowl[current_bowl_position][0][current_bowl_fill_amount];
							break;
						}

						case (LEFT):
						{
							room_entity.sprite = water_bowl[current_bowl_position][1][current_bowl_fill_amount];
							break;
						}

						case (RIGHT):
						{
							room_entity.sprite = water_bowl[current_bowl_position][2][current_bowl_fill_amount];
							break;
						}
					}

				}

				last_arrow 				= current_arrow;
				bowl_position_updated 	= false;
			}

			break;
		}

		case (SALT_COLLECTION):
		{
			if (start_button_hover != start_button_hover_last && !machine_started)
			{
				if (start_button_hover)
					room_entity.sprite = salt_start_button_hover;
				else
					room_entity.sprite = salt_normal;

				start_button_hover_last = start_button_hover;
			}
			else if (machine_started)
			{
				/* Go trough the chat dialogue */
				if (laptop_typing.isPlaying() || mechanical_typing.isPlaying())
					break;

				if (chat_step < 3 && salt_timer.ElapsedSeconds() > 1.5f)
				{
					switch (chat_step)
					{
						case (0):
							laptop_typing.play();
							break;

						case (1):
							mechanical_typing.play();
							break;
					}

					room_entity.sprite = salt_display_on[chat_step];
					chat_step++;
					salt_timer.Start();
				}
				else if (chat_step >= 2 && salt_timer.ElapsedSeconds() > 1.5f && !salt_falling)
				{
					salt_falling = true;
					salt_timer.Start();
				}

				if (salt_falling && salt_falling_step == 0 && !salt_falling_sound.isPlaying())
					salt_falling_sound.play();

				if (salt_falling && salt_falling_step < 10 && salt_timer.ElapsedSeconds() > 0.5f)
				{
					room_entity.sprite = salt_fall_steps[salt_falling_step];
					salt_falling_step++;
					salt_timer.Start();
				}
				else if (salt_falling && salt_falling_step == 10)
				{
					salt_falling = false;
					room_entity.sprite = salt_fall_done;
					salt_timer.Start();
				}
				else if (!salt_collecting_done && !salt_falling && salt_falling_step == 10 && salt_timer.ElapsedSeconds() > 0.35f)
				{
					salt_collecting_done = true;
					room_entity.sprite = salt_return_normal;
					salt_timer.Stop();
				}
				else if (salt_collecting_done)
				{
					if (hovering_over_salt_return != hovering_over_salt_return_last)
					{
						if (hovering_over_salt_return)
							room_entity.sprite = salt_return_hover;
						else
							room_entity.sprite = salt_return_normal;

						hovering_over_salt_return_last = hovering_over_salt_return;
					}
				}
			}
			break;
		}
	}
}

/* render() is called after update() has been finished.
 * Before it gets called, the window will be cleared and
 * after the function has finished running, the rendered frame
 * will be presented */
void render(Game& game)
{
	room.Render();
	crosshair.Render();
	//Render::DrawRect(Colors::Nord::PolarNight::nord0, drag_rect, 2);
}

/* post_render() will be called after rendering has finished
 * and the timestep stalling has started. On non-windows systems
 * this function call will be done on a separate thread, so you
 * could use it to do some extra preparations for the next frame
 * while the main thread is sitting around doing nothing
 * and waiting to maintain the correct frame rate */
void post_render()
{

}

/* cleanup() gets called after the game loop has finished running
 * and the application is getting closed. This is useful for doing any
 * cleanup that is necessary, like freeing heap allocations etc. */
void cleanup()
{
	delete free_font_bold;
}
