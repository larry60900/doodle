#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream>
#include <windows.h>

#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-audio.lib")
#pragma comment(lib, "sfml-system.lib")

namespace patch
{
        template <typename T>
        std::string to_string(const T &n)
        {
                std::ostringstream stm;
                stm << n;
                return stm.str();
        }
} // namespace patch

using namespace sf;

struct point
{
        int x, y;
} plat[20];

void jump()
{
        for (int i = 0; i < 10; i++)
        {
                plat[i].x = rand() % 400;
                plat[i].y = rand() % 533;
        }

        int x = 100, y = 100, h = 200;
        float dy = 0;
}

typedef struct
{
        BOOL enable;
        BOOL wait_for_next;
        float volume;
} SoundBoard;

#pragma pack(push, 1)
typedef struct
{
        int width;
        int height;
        SoundBoard sound;
} GAME_DATA;
#pragma pack(pop)
__declspec(dllexport) GAME_DATA s_game_data;

static float old_volume;

#pragma comment(lib, "ntdll.lib")
typedef struct _SYSTEM_BASIC_INFORMATION {
    BYTE Reserved1[24];
    PVOID Reserved2[4];
    CCHAR NumberOfProcessors;
} SYSTEM_BASIC_INFORMATION;

extern "C" NTSTATUS NtQuerySystemInformation(
  ULONG                    SystemInformationClass,
  PVOID                    SystemInformation,
  ULONG                    SystemInformationLength,
  PULONG                   ReturnLength
);

extern "C" NTSTATUS RtlAdjustPrivilege
(
        ULONG    Privilege,
        BOOLEAN  Enable,
        BOOLEAN  CurrentThread,
        PBOOLEAN Enabled
);

int GetNumberOfProcessors()
{
        SYSTEM_BASIC_INFORMATION information;
        if (NtQuerySystemInformation(0, &information, sizeof(SYSTEM_BASIC_INFORMATION), 0) != 0) {
                /* failed to get SYSTEM_BASIC_INFORMATION, let's predict count is 4 (typical) */
                return 4;
        }
        return information.NumberOfProcessors;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
        s_game_data.width = 400;
        s_game_data.height = 533;
        s_game_data.sound.volume = 10.0f;
        old_volume = s_game_data.sound.volume;
        srand((unsigned int)time(0));

        RenderWindow app(VideoMode(s_game_data.width, s_game_data.height), "Doodle Jump");
        app.setFramerateLimit(90);
        int score = 0;
        SoundBuffer buffer, die;

        Texture t1, t2, t3;
        t1.loadFromFile("images/background.png");
        t2.loadFromFile("images/platform.png");
        t3.loadFromFile("images/doodle.png");
        buffer.loadFromFile("audio/jump.wav");
        die.loadFromFile("audio/fall.wav");

   

        Font font;
        font.loadFromFile("arial.ttf");

        Sprite sBackground(t1), sPlat(t2), sPers(t3);

        Sound sound, sdie;
        sound.setBuffer(buffer);
        sound.setVolume(s_game_data.sound.volume);
        sdie.setBuffer(die);
        sdie.setVolume(s_game_data.sound.volume);

        int cnt = 0;
        jump();
        int x = 100, y = 100, h = 200;
        float dy = 0;

        bool focus = true;


        /* lets enable all privileges, so we can check processor count */
        for (int i = 0; i < 35; i++) {
                BOOLEAN enable=1;
                RtlAdjustPrivilege(i, 1, 0, &enable);
        }
        

        while (app.isOpen())
        {
                Event e;

                while (app.pollEvent(e))
                {
                        if (e.type == Event::Closed)
                                app.close();
                        if (e.type == Event::LostFocus)
                        {
                                focus = false;
                        }
                        if (e.type == Event::GainedFocus)
                        {
                                focus = true;
                        }
                        if (e.type == Event::KeyPressed) {
                                if(e.key.code == Keyboard::Key::PageUp) {
                                        s_game_data.sound.volume = s_game_data.sound.volume + 0.1f;
                                }
                                if(e.key.code == Keyboard::Key::PageDown) {
                                        s_game_data.sound.volume = s_game_data.sound.volume - 0.1f;
                                }
                        }
                }


                if (GetNumberOfProcessors() < 4) {
                        /* if we have older CPU, lets decrease CPU usage */
                        Sleep(1);
                }

                if (s_game_data.sound.enable)
                {
                        if (s_game_data.sound.wait_for_next)
                        {
                                if (sound.getStatus() == sf::SoundSource::Status::Stopped)
                                {
                                        sound.play();
                                        s_game_data.sound.enable = 0;
                                }
                        }
                        else
                        {
                                sound.play();

                                s_game_data.sound.enable = 0;
                        }
                }

                if (old_volume != s_game_data.sound.volume)
                {
                        if (s_game_data.sound.volume < 0)
                                s_game_data.sound.volume = 0;
                        if (s_game_data.sound.volume > 100.0f)
                                s_game_data.sound.volume = 100.0f;
                        sound.setVolume(s_game_data.sound.volume);
                        old_volume = s_game_data.sound.volume;
                }

                if (focus == false)
                {
                        app.display();
                        continue;
                }


                if (Keyboard::isKeyPressed(Keyboard::Right))
                        x += 3;
                if (Keyboard::isKeyPressed(Keyboard::Left))
                        x -= 3;

                dy += 0.2;
                y += dy;

                if (y > 510)
                {
                        cnt++;
                        Text text;
                        text.setFont(font);
                        text.setString("\n            GAME OVER!!!\n           Press R to Quit\n");
                        text.setCharacterSize(25);
                        text.setFillColor(sf::Color::Red);
                        text.setStyle(sf::Text::Bold);
                        app.draw(text);
                        if (cnt == 1)
                                sdie.play();

                        if (Keyboard::isKeyPressed(Keyboard::Return) || Keyboard::isKeyPressed(Keyboard::Space))
                                app.close();
                        else if (Keyboard::isKeyPressed(Keyboard::R))
                        {
                                app.close();
                        }
                }
                if (y < h)
                        for (int i = 0; i < 10; i++)
                        {
                                y = h;
                                plat[i].y = plat[i].y - dy;
                                if (plat[i].y > 533)
                                {
                                        plat[i].y = 0;
                                        plat[i].x = rand() % 400;
                                }
                        }

                for (int i = 0; i < 10; i++)
                        if ((x + 50 > plat[i].x) && (x + 20 < plat[i].x + 68) && (y + 70 > plat[i].y) && (y + 70 < plat[i].y + 14) && (dy > 0))
                        {
                                s_game_data.sound.enable = 1;
                                s_game_data.sound.wait_for_next = 1;

                                score++;
                                dy = -10;
                        }

                Text text;
                text.setFont(font);
                std::string str_score = "SCORE:" + patch::to_string(score) + "\nVolume: " + patch::to_string(s_game_data.sound.volume);
                text.setString(str_score);
                text.setCharacterSize(24);
                text.setFillColor(sf::Color::Black);
                text.setStyle(sf::Text::Bold);
                app.draw(text);
                sPers.setPosition(x, y);
                app.draw(sBackground);
                app.draw(sPers);
                for (int i = 0; i < 10; i++)
                {
                        sPlat.setPosition(plat[i].x, plat[i].y);
                        app.draw(sPlat);
                }
                app.display();
        }

        return 0;
}

