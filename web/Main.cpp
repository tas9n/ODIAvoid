# include <Siv3D.hpp> // OpenSiv3D v0.6.4

void SetGameWindowSize(const Size& size) {
#if SIV3D_PLATFORM(WEB)
	Scene::Resize(size);
#else
	Window::Resize(size);
#endif
}

struct Entity {
	static constexpr Size TextureSize{ 64, 64 };

	Vec2 pos;
	Texture texture;

	Entity(const Vec2& _pos, const Texture& _texture) :
		pos{ _pos },
		texture{ _texture }{

	}

	virtual void draw() const = 0;
	virtual void update() = 0;

	RectF region() const {
		return { pos, TextureSize };
	}

	bool interact(Entity* entity) const {
		return region().intersects(entity->region());
	}
};

struct Player : Entity {
	static constexpr char32_t TextureName[] = U"Entity.Odi";

	Player(const Vec2& pos) : Entity{ pos, TextureAsset(TextureName) } {

	}

	void draw() const override {
		TextureAsset(TextureName).resized(TextureSize).drawAt(pos);
	}

	void update() override {
		pos = Cursor::PosF();
	}
};

struct Enemy : Entity {
	static constexpr char32_t TextureName[] = U"Entity.Enemy";
	static constexpr int32 RetargetTime = 500;

	static constexpr int32 MoveSpeed = 300;

	Stopwatch retargetTimer{ StartImmediately::Yes };
	Entity* target;
	Vec2 direction;

	Enemy(const Vec2& pos, Entity* _target) : Entity{ pos, TextureAsset(TextureName) }, target{ _target } {
		retarget();
	}

	void draw() const override {
		TextureAsset(TextureName).resized(TextureSize).drawAt(pos);
	}

	void update() override {
		if (retargetTimer.ms() >= RetargetTime) {
			retargetTimer.restart();
			retarget();
		}

		pos += direction * MoveSpeed * Scene::DeltaTime();
	}

	void retarget() {
		direction = (target->pos - pos).normalized();
	}
};

void Main(){
	constexpr int32 SpawnFrequency = 100;

	TextureAsset::Register(Player::TextureName, U"asset/img/odi.png");
	TextureAsset::Register(Enemy::TextureName, U"asset/img/odi.png");

	SetGameWindowSize(Size{ 920, 920 });
	Scene::SetBackground(Palette::Black);

	Player player{ Cursor::PosF() };
	Array<Enemy> enemies;

	Stopwatch emenySpawner{ StartImmediately::Yes };

	bool pause = true;

	bool gameover = false;

	while (System::Update()) {
		if (pause) {
			if(not emenySpawner.isPaused()) emenySpawner.pause();

			SimpleGUI::GetFont()(U"Pause").drawAt(Scene::Center());
			if (MouseL.down()) {
				pause = false;
				emenySpawner.start();
			}
			continue;
		}

		if (gameover) {
			Print << U"Gameover!";
		}

		if (emenySpawner.ms() >= SpawnFrequency) {
			emenySpawner.restart();

			int32 spawnDirection = Random<int32>(0, 3);

			Vec2 spawnPosition;
			if (spawnDirection == 0 || spawnDirection == 1) {
				spawnPosition = { Random(Scene::Width()), Scene::Height() * spawnDirection };
			}
			if (spawnDirection == 2 || spawnDirection == 3) {
				spawnPosition = { Scene::Width() * (spawnDirection - 2), Random(Scene::Height())};
			}


			enemies << Enemy{
				spawnPosition,
				&player
			};
		}

		for (auto&& enemy : enemies) {
			enemy.update();

			if (enemy.interact(&player)) {
				gameover = true;
			}

			enemy.draw();
		}

		player.update();
		player.draw();
	}
}
