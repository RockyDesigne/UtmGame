﻿// TestGame.cpp : Defines the entry point for the application.
//

#include "TestGame.h"
#include "IItem.h"
#include "Player.h"
#include "Tile.h"
#include "TileMap.h"
#include "InputHandler.h"
#include "ActionHandler.h"

namespace Raylib {
#include <raylib.h>
#include <raymath.h>
}

using namespace Raylib; // ar fi chill sa il bagam intr-un namespace pt a evita naming conflicts

#include <vector>
#include <memory>

constexpr int screenWidth = 800;
constexpr int screenHeight = 800;

bool check_if_player_on_item(const std::shared_ptr<Tile> tile, const Player& player)
{
	auto item = tile->get_item();
	if (item == nullptr)
	{
		return false;
	}

	// Use the actual item size for the bounding box
	Vector2 item_pos = item->get_position();
	Vector2 item_size = item->get_size(); // Assuming get_size() is implemented

	Rectangle item_rec = { item_pos.x, item_pos.y, item_size.x, item_size.y };

	// Adjust player's bounding box to be centered
	Rectangle player_rec = {
		player.m_position.x,
		player.m_position.y,
		player.m_size.x,
		player.m_size.y
	};

	return CheckCollisionRecs(player_rec, item_rec);
	
}

bool can_step(GameActor& player, TileMap& tile_map)
{
	auto player_pos = player.get_position();
	auto player_size = player.get_size();
	auto player_rectangle = Rectangle{
		player_pos.x,
		player_pos.y,
		player_size.x,
		player_size.y };

	auto tile_pos = tile_map.get_tile_pos_in_grid(player_size, player_pos);

	if (!tile_map.is_in_map(tile_pos))
	{
		return false;
	}

	//get the tiles adjacent to the player
	for (int y = -1; y <= 1; y++)
	{
		for (int x = -1; x <= 1; x++)
		{
			Raylib::Vector2 new_tile_pos = { tile_pos.x + x, tile_pos.y + y };

			Raylib::Rectangle tile_rect = {

					new_tile_pos.x * tile_map.m_tile_size.x,
					new_tile_pos.y * tile_map.m_tile_size.y,
					tile_map.m_tile_size.x,
					tile_map.m_tile_size.y
			};

			//TODO: Add concept of a void tile basically to make this prettier
			if (!tile_map.is_in_map(new_tile_pos))
			{
				if (Raylib::CheckCollisionRecs(
					player_rectangle,
					tile_rect)
					)
				{
					return false;
				}
				continue;
			}

			auto tile = tile_map.get_tile_type({ tile_pos.x + x, tile_pos.y + y });

			if (tile == TileType::WALL)
			{
				if (Raylib::CheckCollisionRecs(
					player_rectangle,
					tile_rect)
					)
				{
					return false;
				}
			}
		}
	}

	return true;
}

int main()
{
	//functia asta trebuie apelata mereu la inceput
	InitWindow(screenWidth, screenHeight, "TestGame"); //idk ne trebuie un nume

	SetWindowState(FLAG_VSYNC_HINT); //aici putem avea niste flags pt a seta chestii despre windo

	ClearBackground(Raylib::BLACK);

	TileMap tile_map { {20.f,20.f}, {5.f,5.f} };

	tile_map.load_map(R"(G:\projects\repos\TestGame\res\map.txt)");

	tile_map.init();

	Player player{{},{15.f,15.f},Raylib::RED};

	player.m_position = tile_map.get_player_position(player.m_size);

	SetTargetFPS(60); 

	InputHandler input_handler{new MoveUpCommand, new MoveLeftCommand, new MoveDownCommand, new MoveRightCommand, new MouseLeftClickCommand};

	ActionHandler action_handler;

	Camera2D camera = { };
	camera.target = { player.m_position };
	camera.offset = { static_cast<float>(screenWidth) / 2,
		static_cast<float>(screenHeight) / 2 };
	camera.rotation = 0.0f;
	camera.zoom = 3.0f;

	while (!WindowShouldClose())
	{

		if (Raylib::IsKeyPressed(Raylib::KEY_F5))
		{
			tile_map.load_map(R"(G:\projects\repos\TestGame\res\map.txt)");
			tile_map.init();
			player.m_position = tile_map.get_player_position(player.m_size);
		}

		auto prev_pos = player.m_position;

		auto command = input_handler.handle_input();

		if (command)
		{
			command->execute(player);
		}

		if (!can_step(player, tile_map))
		{
			player.m_position = prev_pos;
		}

		auto tile = tile_map.get_tile_object(tile_map.get_tile_pos_in_grid(
			player.m_size,
			player.m_position));
		if (check_if_player_on_item(tile, player))
		{
			auto item = tile->get_item();
			if (item)
			{
				player.m_items.push_back(item);
				tile->remove_item();
			}
		}

		auto player_tile = tile_map.get_tile_pos_in_grid(
			player.m_size,
			player.m_position);

		printf("Player is on tile: x: %f, y: %f\n", player_tile.x, player_tile.y);

		camera.target = { player.m_position }; //camera urmareste playerul

		// Camera zoom controls
		camera.zoom += (GetMouseWheelMove() * 0.05f);

		//limitam zoomul
		if (camera.zoom > 3.0f) camera.zoom = 3.0f;
		else if (camera.zoom < 0.1f) camera.zoom = 0.1f;

		BeginDrawing(); //tot ce este intre astea doua se intampla de 60 de ori pe secunda
			ClearBackground(Raylib::BLACK); //trb every frame

			BeginMode2D(camera);

				tile_map.draw();

				player.draw_player();

				//drawing player actions
				//TODO: Think about actions that are executed over several frames
				if (!player.m_actions.empty()) {
					action_handler.handle_action(player.m_actions.front(), player);
					player.m_actions.pop();
				}

			EndMode2D();

		EndDrawing();
			
	}

	CloseWindow();

	return 0;
}
