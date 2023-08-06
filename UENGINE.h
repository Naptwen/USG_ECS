//"Please refer to the licenses for SDL2.0"

/* EXAMPLE FOR 2D SDL ENGINE 1.1.0 */
/* USG (c) July 16th, 2023.
Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __UENGINE_H__
#define __UENGINE_H__
#include "USDL.h"
#include "UECS.h"

using namespace usdl;
using namespace uecs;


namespace UENGINE
{
	struct Trigger
	{
		bool trigger = false;
	};

	struct Object
	{
		const char* name;

		Object(void) = default;
		Object(const char* name) : name(name) {}
		Object& operator = (Object const& other)
		{
			name = other.name;
		}
	};

	struct Position
	{
		unsigned int px;
		unsigned int py;

		Position(void) = default;
		Position(unsigned int px, unsigned int py) : px(px), py(py) {}
		Position& operator = (Position& other)
		{
			px = other.px;
			py = other.py;
		}
	};

	Entity& CreateEmptyObject(World& mWorld, const char* name, const unsigned int px, const unsigned int py)
	{
		return mWorld.entity(name)
			.set<Object>({ name })
			.set<Position>({ px, py });
	}

	void AddImageObject(Entity& entity, Window& mWindow, const char* path, int rows, int cols, int indexX, int indexY, int sizeX, int sizeY, int posX, int posY)
	{
		entity.set<Image>({});
		auto img = entity.get<Image>();
		img->Init(mWindow);
		img->Load(path);
		img->Split(rows, cols);
		img->Index(indexX, indexY);
		img->Size(sizeX, sizeY);
		img->Pose(posX, posY);
	}

	void SetAnimation2Object(Entity& entity, bool loop, int row)
	{
		auto img = entity.get<Image>();
		img->SetAnim(loop, row);
	}

	void AddTextObject(Entity& entity, Window& mWindow, const char* text, const char* font, int font_sz)
	{
		entity.set<Text>({});
		auto txt = entity.get<Text>();
		txt->Init(mWindow);
		txt->Font(font, font_sz, { 255, 255, 255 });
		txt->Write(text);
		txt->Position(0, 0);
	}

	void AddSoundObject(Entity& entity, const char* path, usdl::Music::TYPE type)
	{
		entity.set<Music>({});
		auto sound = entity.get<Music>();
		sound->Init(path, type);
	}

	void Drawing(Entity& entity)
	{
		if (entity.isHas<Image>())
			entity.get<Image>()->Draw();
	}

	void Printing(Entity& entity)
	{
		if (entity.isHas<Text>())
			entity.get<Text>()->Draw();
	}

	void SoundPlaying(Entity& entity)
	{
		if (entity.isHas<Music>()){
			entity.get<Music>()->Play();
			entity.remove<Trigger>();
		}
	}
}

struct GAME_WINDOW
{
	Window mWindow;
	World mWorld;

	GAME_WINDOW(int w, int h)
	{
		mWindow = Window();
		mWindow.Init(w, h);
	}

	Window& getWindow() { return mWindow; }
	World& getWorld() { return mWorld; }

	void GameLoop(void)
	{
		if (mWorld.entity("TORNADO").get<Image>()->IsAnimFinish() && mWorld.entity("TORNADO").get<Image>()->IsEnable())
		{
			mWorld.entity("TORNADO").get<Image>()->Disable();
			mWorld.entity("TORNADO").get<Image>()->ResetFrame();
		}
		auto mouse = mWindow.GetMouse(SDL_BUTTON_LEFT);
		if (mouse.z)
		{
			auto entity_list = mWorld.find<UENGINE::Object>();
			for (auto& entity : entity_list)
			{
				if(!std::strcmp(entity.get<UENGINE::Object>()->name, "MUSIC"))
				{
					entity.set<UENGINE::Trigger>({});
				}					
			}
			if (!mWorld.entity("TORNADO").get<Image>()->IsEnable())
			{
				int drawszW, drawszH;
				mWorld.entity("TORNADO").get<Image>()->GetDrawSize(drawszW, drawszH);
				ULOG(UWARNING, "%d %d", drawszW, drawszH);
				mWorld.entity("TORNADO").get<Image>()->Pose(mouse.x - drawszW * 0.5, mouse.y - drawszH * 0.5);
				mWorld.entity("TORNADO").get<Image>()->Enable();
			}
		}
	}
	void RunGameMode(void)
	{
		ULOG(UERROR, "START THE GAME MODE");
		mWorld.system<UENGINE::Object>(UENGINE::Drawing, uecs::PHASE::Update);
		mWorld.system<UENGINE::Object>(UENGINE::Printing, uecs::PHASE::OffUpdate);
		mWorld.system<UENGINE::Trigger>(UENGINE::SoundPlaying, uecs::PHASE::OffUpdate);
		mWorld.settingTimer(30, 100);
		while (true)
		{

			mWindow.Clear();
			mWindow.UpdateKey();
			mWorld.update_progress();
			SDL_GL_SwapWindow(mWindow.getWindow());
			GameLoop();
			if (mWindow.IsExit()) break;
		}
	}
};

#endif
