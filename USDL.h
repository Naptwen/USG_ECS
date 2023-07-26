#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <unordered_map>
#include <queue>
#include <string>
#include <codecvt>

#define UTF8(txt) u8##txt

namespace usdl
{
	struct Vec2i
	{
		int  x;
		int  y;
	};

	struct Vec3i
	{
		int x;
		int y;
		int z;

		Vec3i() = default;

		Vec3i(int x, int y, int z) : x(x), y(y), z(z) {}

		Vec3i(Vec3i const& other) : x(other.x), y(other.y), z(other.z) {}

		Vec3i (Vec3i&& other) noexcept
		{
			x = std::exchange(other.x, 0);
			y = std::exchange(other.y, 0);
			z = std::exchange(other.z, 0);
		}

		Vec3i& operator = (Vec3i const& other)
		{
			return *this = Vec3i(x, y, z);
		}

		Vec3i& operator = (Vec3i&& other) noexcept
		{
			x = std::exchange(other.x, 0);
			y = std::exchange(other.y, 0);
			z = std::exchange(other.z, 0);
			return *this;
		}
	};

	using AniFrame = std::vector<Vec2i>;

	struct Window
	{
	private:
		int w = 100;
		int h = 100;
		SDL_Renderer* renderer = nullptr;
		SDL_Window* window = nullptr;
		std::unordered_map<SDL_Keycode, bool> keyMap;
		std::unordered_map<Uint8, Vec3i> mouseMap;
		bool Exit = false;
	public:
		Window(void) {}

		Window(int w, int h) : w(w), h(h) { Init(w, h); }

		SDL_Renderer* getRenderer(){ return renderer; }

		SDL_Window* getWindow() { return window; }

		void Init(int _w, int _h)
		{
			w = _w;
			h = _h;
			SDL_Init(SDL_INIT_EVERYTHING);
			int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_JXL | IMG_INIT_AVIF;
			IMG_Init(imgFlags);
			TTF_Init();
			Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
			CreateWindow();
		}

		void CreateWindow()
		{
			window = SDL_CreateWindow("Hello SDL Developer!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, 0);
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		}

		void Clear()
		{
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		}

		void UpdateKey()
		{
			keyMap.clear();
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT) {
					Exit = true;
					break;
				}
				if (event.type == SDL_KEYDOWN)
				{
					SDL_Keycode keycode = event.key.keysym.sym;
					keyMap[keycode] = true;
				}
				if (event.type == SDL_MOUSEBUTTONDOWN)
				{
					SDL_MouseButtonEvent keycode = event.button;
					int x, y;
					SDL_GetMouseState(&x, &y);
					mouseMap[event.button.button].x = x;
					mouseMap[event.button.button].y = y;
					mouseMap[event.button.button].z = true;
				}
			}
		}

		bool IsExit()
		{
			return Exit;
		}

		Vec3i GetMouse(Uint8 code)
		{
			if(mouseMap.count(code) > 0)
			return std::exchange(mouseMap[code], Vec3i());
			return Vec3i();
		}

		bool GetKey(SDL_KeyCode code)
		{
			return std::exchange(keyMap[code], false);
		}

		void Present()
		{
			SDL_RenderPresent(renderer);
		}

		void Destroy()
		{
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
			IMG_Quit();
			TTF_Quit();
			Mix_CloseAudio();
			SDL_Quit();
			renderer = nullptr;
			window = nullptr;
		}

		void reset()
		{
			Exit = false;
			keyMap.clear();
			Destroy();
			CreateWindow();
		}

		~Window()
		{
			Destroy();
		}
	};

	struct Image
	{
	private:
		mutable SDL_Renderer* _renderer = nullptr;
		mutable SDL_Window* _window = nullptr;
		mutable const char* path = NULL;
		mutable SDL_Texture* texture = NULL;
		mutable SDL_Surface* surface = NULL;
		SDL_Rect OriginSize = { 0, 0, 0, 0 };
		SDL_Rect ImageSplite = { 0, 0, 0, 0 };
		SDL_Rect DrawSize = { 0, 0, 0, 0 };
		AniFrame Animation;
		Vec2i split = { 0, 0 };
		int _frame = 0;
		bool _enable = true;
		bool _loop = true;
	public:
		Image(void) = default;
		Image(SDL_Renderer* _renderer, 
			SDL_Window* _window, 
			const char* path, 
			SDL_Texture* texture, 
			SDL_Surface* surface, 
			SDL_Rect OriginSize, 
			SDL_Rect ImageSplite, 
			SDL_Rect DrawSize, 
			AniFrame Animation,
			Vec2i split, 
			bool _enable,
			bool _loop) :
			_renderer(_renderer),
			_window(_window), 
			path(path), 
			texture(texture),
			surface(surface), 
			OriginSize(OriginSize), 
			ImageSplite(ImageSplite),
			DrawSize(DrawSize), 
			Animation(Animation), 
			split(split), 
			_enable(_enable),
			_loop(_loop) {}
		Image(Image const& other):
			Image(	other._renderer, 
					other._window, 
					other.path, 
					other.texture, 
					other.surface, 
					other.OriginSize, 
					other.ImageSplite, 
					other.DrawSize, 
					other.Animation, 
					other.split, 
					other._enable,
					other._loop) {}
		Image(Image&& other) noexcept
		{
			_renderer = std::move(other._renderer);
			_window = std::move(other._window);
			path = std::move(other.path);
			texture = std::move(other.texture);
			surface = std::move(other.surface);
			OriginSize = other.OriginSize;
			ImageSplite = other.ImageSplite;
			DrawSize = other.DrawSize;
			Animation = other.Animation;
			split = other.split;
			_enable = other._enable;
			_loop = other._loop;
		}
		Image& operator = (Image const& other)
		{
			_renderer = std::move(other._renderer);
			_window = std::move(other._window);
			path = std::move(other.path);
			texture = std::move(other.texture);
			surface = std::move(other.surface);
			OriginSize = other.OriginSize;
			ImageSplite = other.ImageSplite;
			DrawSize = other.DrawSize;
			Animation = other.Animation;
			split = other.split;
			_enable = other._enable;
			_loop = other._loop;
			return *this;
		}
		Image& operator = (Image&& other) noexcept 
		{
			return *this = Image(other);
		}

		void Set(Window& window)
		{
			_renderer = window.getRenderer();
			_window = window.getWindow();
		}
		void Load(const char* path)
		{
			surface = IMG_Load(path);
			SDL_assert(surface);
			texture = SDL_CreateTextureFromSurface(_renderer, surface);
			SDL_assert(texture);
			SDL_FreeSurface(surface);
			SDL_QueryTexture(texture, NULL, NULL, &OriginSize.w, &OriginSize.h);
			ImageSplite.w = OriginSize.w;
			ImageSplite.h = OriginSize.h;
		}
		void Split(const int rows, const int cols)
		{
			SDL_assert(rows != 0 && cols != 0);
			split = { rows, cols };
			ImageSplite.h = OriginSize.h / rows;
			ImageSplite.w = OriginSize.w / cols;
		}
		void Index(int row, int col)
		{
			SDL_assert(split.x != 0 && split.y != 0);
			ImageSplite.x = ImageSplite.w * col;
			ImageSplite.y = ImageSplite.h * row;
		}
		void Size(const int w, const int h)
		{
			DrawSize.w = w;
			DrawSize.h = h;
		}
		void Pose(int px, int py)
		{
			DrawSize.x = px;
			DrawSize.y = py;
		}
		void Anim(bool loop, int index_row = 0)
		{
			SDL_assert(split.x != 0 && split.y != 0);
			_loop = loop;
			for (int j = 0; j < split.y; ++j)
			{
				Animation.emplace_back(Vec2i({ index_row, j }));
			}
		}
		bool IsAnimFinish()
		{
			SDL_assert(!Animation.empty());
			return (Animation.size() == _frame);
		}
		void Draw()
		{
			if (!_enable) return;
			SDL_assert(texture);
			if (Animation.empty())
			{
				SDL_RenderCopy(_renderer, texture, &ImageSplite, &DrawSize);
			}
			if (_loop && _frame >= Animation.size())
			{
				_frame = 0;
			}
			if (_frame < Animation.size())
			{
				int x = Animation[_frame].x;
				int y = Animation[_frame].y;
				Index(x, y);
				SDL_RenderCopy(_renderer, texture, &ImageSplite, &DrawSize);
				_frame++;
			}
		}
		void Disable()
		{
			_enable = false;
		}
		void Enable()
		{
			_enable = true;
		}
		bool IsEnable() { return _enable; }
		void GetSize(int& w, int& h)
		{
			w = DrawSize.w;
			h = DrawSize.h;
		}
		int GetFrame() {return _frame; }
		void ResetFrame() { _frame = 0; }
		~Image()
		{
			SDL_DestroyTexture(texture);
		}
	};
	struct Text
	{
		SDL_Renderer* _renderer = nullptr;
		SDL_Window* _window = nullptr;
		TTF_Font* _font = nullptr;
		SDL_Texture* _textTexture = nullptr;
		SDL_Color _textColor = {0, 0, 0};
		SDL_Rect _textRect = {0, 0, 0, 0};
		const char* _text = nullptr;
		bool _enable = true;
		Text(void) = default;
		Text(SDL_Renderer* _renderer, 
			SDL_Window* _window, 
			TTF_Font* _font, 
			SDL_Texture* _textTexture, 
			SDL_Color _textColor,
			SDL_Rect _textRect, 
			const char* _text,
			bool _enable)
		:	_renderer(_renderer), 
			_window(_window), 
			_font(_font), 
			_textTexture(_textTexture),
			_textColor(_textColor), 
			_textRect(_textRect), 
			_text(_text),
			_enable(_enable) {}
		Text(Text const& other) 
		: Text(	other._renderer, 
				other._window, 
				other._font, 
				other._textTexture, 
				other._textColor, 
				other._textRect, 
				other._text,
				other._enable) {}
		Text(Text&& other) noexcept 
		{
			_renderer = std::move(other._renderer);
			_window = std::move(other._window);
			_font = std::move(other._font);
			_textTexture = std::move(other._textTexture);
			_textColor = other._textColor;
			_textRect = other._textRect;
			_text = std::move(other._text);
		}
		Text& operator = (Text const& other)
		{			
			return *this = Text(other);
		}
		Text& operator = (Text&& other) noexcept
		{
			_renderer = std::move(other._renderer);
			_window = std::move(other._window);
			_font = std::move(other._font);
			_textTexture = std::move(other._textTexture);
			_textColor = other._textColor;
			_textRect = other._textRect;
			_text = std::move(other._text);
			_enable = other._enable;
			return *this;
		}

		void Set(Window& window)
		{
			_renderer = window.getRenderer();
			_window = window.getWindow();
		}
		void Font(const char* text, const char* font_path, unsigned int font_size, SDL_Color color)
		{
			_font = TTF_OpenFont(font_path, font_size);
			_text = text;
			_textColor = color;
			_textRect.w = static_cast<int>(std::strlen(text) * ((font_size) >> 1));
			_textRect.h = font_size;
		}
		void Position(int x, int y)
		{
			_textRect.x = x;
			_textRect.y = y;
		}
		void Draw()
		{			
			if(!_enable) return;
			SDL_assert(_renderer);
			SDL_Surface* textSurface = TTF_RenderUTF8_Solid(_font, _text, _textColor);
			SDL_assert(textSurface);
			_textTexture = SDL_CreateTextureFromSurface(_renderer, textSurface);
			SDL_RenderCopy(_renderer, _textTexture, NULL, &_textRect);
			SDL_FreeSurface(textSurface);
			SDL_DestroyTexture(_textTexture);
		}
		void Disable()
		{
			_enable = false;
		}
		void Enable()
		{
			_enable = true;
		}
		bool IsEnable() { return _enable; }

		~Text()
		{
			TTF_CloseFont(_font);
		}
	};
	struct Music
	{
		enum class TYPE
		{
			music,
			effect
		};

		TYPE _type;
		Mix_Music* _music = nullptr;
		Mix_Chunk* _chunk = nullptr;
		bool _enable = true;

		Music() = default;
		Music(TYPE _type, Mix_Music* _music, Mix_Chunk* _chunk, bool _enable): _type(_type), _music(_music), _chunk(_chunk), _enable(_enable) {};
		Music(Music const& other) : Music(other._type, other._music, other._chunk, other._enable) {}
		Music(Music&& other) noexcept
		{
			_type = std::move(other._type);
			_music = std::move(other._music);
			_chunk = std::move(other._chunk);
			_enable = other._enable;
		}
		Music& operator = (Music const& other)
		{
			return *this = Music(other);
		}
		Music& operator = (Music&& other) noexcept
		{
			_type = other._type;
			_music = std::move(other._music);
			_chunk = std::move(other._chunk);
			_enable = other._enable;
			return *this;
		}

		void Set(const char* music_path, TYPE type)
		{
			_type = type;
			if (_type == TYPE::music)
			_music = Mix_LoadMUS(music_path);
			if (_type == TYPE::effect)
			_chunk = Mix_LoadWAV(music_path);
		}
		void Play()
		{
			if(!_enable) return;
			if(_type == TYPE::music)
				Mix_PlayMusic(_music, -1);
			if (_type == TYPE::effect)
				Mix_PlayChannel(-1, _chunk, 0);
		}
		void Disable()
		{
			_enable = false;
		}
		void Enable()
		{
			_enable = true;
		}
		bool IsEnable() { return _enable; }

		~Music()
		{
			if(_music)
				Mix_FreeMusic(_music);
			if(_chunk)
				Mix_FreeChunk(_chunk);
		}
	};
}
