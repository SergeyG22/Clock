#define _CRT_SECURE_NO_WARNINGS

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <chrono>
#include <iomanip>



// Do we want to showcase direct JNI/NDK interaction?
// Undefine this to get real cross-platform code.
// Uncomment this to try JNI access; this seems to be broken in latest NDKs
//#define USE_JNI

#if defined(USE_JNI)
// These headers are only needed for direct NDK/JDK interaction
#include <jni.h>
#include <android/native_activity.h>

// Since we want to get the native activity from SFML, we'll have to use an
// extra header here:
#include <SFML/System/NativeActivity.hpp>

// NDK/JNI sub example - call Java code from native code
int vibrate(sf::Time duration)
{
    // First we'll need the native activity handle
    ANativeActivity *activity = sf::getNativeActivity();
    
    // Retrieve the JVM and JNI environment
    JavaVM* vm = activity->vm;
    JNIEnv* env = activity->env;

    // First, attach this thread to the main thread
    JavaVMAttachArgs attachargs;
    attachargs.version = JNI_VERSION_1_6;
    attachargs.name = "NativeThread";
    attachargs.group = NULL;
    jint res = vm->AttachCurrentThread(&env, &attachargs);

    if (res == JNI_ERR)
        return EXIT_FAILURE;

    // Retrieve class information
    jclass natact = env->FindClass("android/app/NativeActivity");
    jclass context = env->FindClass("android/content/Context");
    
    // Get the value of a constant
    jfieldID fid = env->GetStaticFieldID(context, "VIBRATOR_SERVICE", "Ljava/lang/String;");
    jobject svcstr = env->GetStaticObjectField(context, fid);
    
    // Get the method 'getSystemService' and call it
    jmethodID getss = env->GetMethodID(natact, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject vib_obj = env->CallObjectMethod(activity->clazz, getss, svcstr);
    
    // Get the object's class and retrieve the member name
    jclass vib_cls = env->GetObjectClass(vib_obj);
    jmethodID vibrate = env->GetMethodID(vib_cls, "vibrate", "(J)V"); 
    
    // Determine the timeframe
    jlong length = duration.asMilliseconds();
    
    // Bzzz!
    env->CallVoidMethod(vib_obj, vibrate, length);

    // Free references
    env->DeleteLocalRef(vib_obj);
    env->DeleteLocalRef(vib_cls);
    env->DeleteLocalRef(svcstr);
    env->DeleteLocalRef(context);
    env->DeleteLocalRef(natact);
    
    // Detach thread again
    vm->DetachCurrentThread();
}
#endif

// This is the actual Android example. You don't have to write any platform
// specific code, unless you want to use things not directly exposed.
// ('vibrate()' in this example; undefine 'USE_JNI' above to disable it)



class Clock:public sf::Drawable
{	
	float x;
	float y;
	sf::Texture t_clock;
	sf::Texture t_point;
	sf::Texture t_second;
	sf::Texture t_minute;
	sf::Texture t_hour;
	sf::SoundBuffer buffer;	
	sf::Sprite s_point;
	sf::Sprite s_second;
	sf::Sprite s_minute;
	sf::Sprite s_hour;	
public:
	virtual void draw(sf::RenderTarget&, sf::RenderStates)const;
	void arrow_rotation();
	void initialization_by_the_system_clock();
	Clock();
	sf::Sprite s_clock;
	sf::Sound sound;
};

void Clock::arrow_rotation()
{
	s_hour.rotate(0.008333333);
	s_second.rotate(6);
	s_minute.rotate(0.1);
	sound.play();
}



void Clock::draw(sf::RenderTarget& target, sf::RenderStates states)const
{
	target.draw(s_clock);
	target.draw(s_point);
	target.draw(s_second);
	target.draw(s_minute);
	target.draw(s_hour);
}

void Clock::initialization_by_the_system_clock()
{
	auto now = std::chrono::system_clock::now();
	auto my_time = std::chrono::system_clock::to_time_t(now);
	s_second.setRotation((localtime(&my_time)->tm_sec * 6));
	s_minute.setRotation((localtime(&my_time)->tm_min * 6));
	s_hour.setRotation(((localtime(&my_time)->tm_hour - 12) * 30) + ((localtime(&my_time)->tm_min) * 0.5));

}

Clock::Clock() :x(0), y(400)
{

	t_clock.loadFromFile("clock.png");
	s_clock.setTexture(t_clock);
	s_clock.setPosition(x, y);

	t_point.loadFromFile("point.png");
	s_point.setTexture(t_point);
	s_point.setPosition(360, 750);

	t_second.loadFromFile("second.png");
	s_second.setTexture(t_second);
	s_second.setOrigin(10, 215);
	s_second.setPosition(370, 760);

	t_minute.loadFromFile("minute.png");
	s_minute.setTexture(t_minute);
	s_minute.setOrigin(10, 220);
	s_minute.setPosition(370, 760);

	t_hour.loadFromFile("hour.png");
	s_hour.setTexture(t_hour);
	s_hour.setOrigin(32, 182);
	s_hour.setPosition(370, 760);
	
	initialization_by_the_system_clock();

	buffer.loadFromFile("tick.wav");
	sound.setBuffer(buffer);
}

class Pendulum_clock:public sf::Drawable
{
	float x;
	float y;
	sf::Texture t_clock;
	sf::Texture t_second;
	sf::Texture t_minute;
	sf::Texture t_hour;
	sf::Texture t_pendulum;	
	sf::Sprite s_second;
	sf::Sprite s_minute;
	sf::Sprite s_hour;	
	sf::SoundBuffer buffer;
public:
	Pendulum_clock();
	virtual void draw(sf::RenderTarget&, sf::RenderStates)const;
	void initialization_by_the_system_clock();
	void arrow_rotation();
	void pendulum_swing();
	void mouse_event(sf::RenderWindow&,sf::Event&);
	void sensor_event(sf::RenderWindow&,sf::Event&);
	sf::Sound sound;
	sf::Sprite s_clock;
	sf::Sprite s_pendulum;
	bool check = true;
	int a = 0;
	float angle_value = 210; //счетчик-переменная служит мыслимым диапазоном [0-120-240] в котором качается маятник
};

void Pendulum_clock::arrow_rotation()
{
	if (check)
	{
		s_second.rotate(6);
		s_minute.rotate(0.1);
		s_hour.rotate(0.008333333);
		sound.play();
	}
	
}

void Pendulum_clock::pendulum_swing()
{
	if (check)
	{
		switch (a)
		{
		case 0:  angle_value += 8.2; s_pendulum.rotate(0.5); if (angle_value > 420) { a = 1; } break;
		case 1:  angle_value -= 8.2; s_pendulum.rotate(-0.5); if (angle_value < 0) { a = 0; } break;
		} 
	}

}
// angle_value изменено с 4.2 на компьютере на 8.2 на сенсоре (так как скорость процессоров разная)

void Pendulum_clock::mouse_event(sf::RenderWindow& w,sf::Event& e)
{
	sf::Vector2i get_pixel = sf::Mouse::getPosition(w);
	sf::Vector2f position = w.mapPixelToCoords(get_pixel);

			if (e.type == sf::Event::MouseButtonPressed)
			{
				if (e.key.code == sf::Mouse::Left)
				{
					if (s_pendulum.getGlobalBounds().contains(position)) { check = false;}
				}
			}

			if (e.type == sf::Event::MouseButtonReleased)
			{
				if (e.key.code == sf::Mouse::Left)
				{
					if (s_pendulum.getGlobalBounds().contains(position)) { check = true; initialization_by_the_system_clock();
					}
				}
			}			
}

void Pendulum_clock::sensor_event(sf::RenderWindow& w, sf::Event& e)
{
	sf::Vector2i get_pixel = sf::Touch::getPosition(0,w);
	sf::Vector2f pos = w.mapPixelToCoords(get_pixel);

	if (e.type == sf::Event::TouchBegan)
	{		
		if (s_pendulum.getGlobalBounds().contains(pos)) { check = false;}
	}
	    if (e.type == sf::Event::TouchEnded) { check = true; initialization_by_the_system_clock();}
}




void Pendulum_clock::draw(sf::RenderTarget& target, sf::RenderStates states)const
{
	target.draw(s_pendulum);
	target.draw(s_clock);		
	target.draw(s_minute);
	target.draw(s_hour);
	target.draw(s_second);
}

void Pendulum_clock::initialization_by_the_system_clock()
{
	auto now = std::chrono::system_clock::now();
	auto my_time = std::chrono::system_clock::to_time_t(now);
	s_second.setRotation((localtime(&my_time)->tm_sec * 6));
	s_minute.setRotation((localtime(&my_time)->tm_min * 6));
	s_hour.setRotation(((localtime(&my_time)->tm_hour - 12) * 30) + ((localtime(&my_time)->tm_min) * 0.5));
}


Pendulum_clock::Pendulum_clock():x(80),y(350)
{

	t_pendulum.loadFromFile("pendulum_versia2.png");
	s_pendulum.setTexture(t_pendulum);
	s_pendulum.setOrigin(75, 0);
	s_pendulum.setPosition(363,1010);

	t_clock.loadFromFile("clock_versia2.png");
	s_clock.setTexture(t_clock);
	s_clock.setPosition(x, y);

	t_second.loadFromFile("second_versia2.png");
	s_second.setTexture(t_second);
	s_second.setOrigin(20, 195);
	s_second.setPosition(363, 760);

	t_minute.loadFromFile("minute_versia2.png");
	s_minute.setTexture(t_minute);
	s_minute.setOrigin(20, 245);
	s_minute.setPosition(363, 760);

	buffer.loadFromFile("tick2.wav");
	sound.setBuffer(buffer);

	t_hour.loadFromFile("hour_versia2.png");
	s_hour.setTexture(t_hour);
	s_hour.setOrigin(20, 220);
	s_hour.setPosition(363, 760);
	initialization_by_the_system_clock();
}



int main()
{
	sf::RenderWindow window(sf::VideoMode(720, 1520), "Clock");
	// sf::VideoMode(720, 1520),
	Clock clock;
	Pendulum_clock clock_v1;
	sf::Time time = sf::seconds(0.01f);	
	sf::Clock simple_timer;
	int current_screen(0);

	

	while (window.isOpen())
	{
		sf::Event event;
		time = simple_timer.getElapsedTime();	
		
		clock_v1.pendulum_swing();

		if ((int)time.asSeconds() == 1)
		{
			
			switch (current_screen)
			{
			case 0:clock.arrow_rotation();
				break;
			case 1:clock_v1.arrow_rotation(); 
				break;
			}

			simple_timer.restart();
		}
		
		while (window.pollEvent(event))
		{
			sf::Vector2i get_pixel = sf::Mouse::getPosition(window);
			sf::Vector2f position = window.mapPixelToCoords(get_pixel);
						
			if (event.type == sf::Event::MouseButtonPressed)
			{
				if (event.key.code == sf::Mouse::Left)
				{					
					if (clock.s_clock.getGlobalBounds().contains(position))
					{
						++current_screen;
						if (current_screen > 1){ current_screen = 0; }
						  
						switch (current_screen)
						{
						case 0:clock.initialization_by_the_system_clock(); clock_v1.sound.stop(); break;
						case 1:clock_v1.initialization_by_the_system_clock(); clock.sound.stop(); break;
						}
					}  					
				}			
			}
			

			sf::Vector2i touch_pixel = sf::Touch::getPosition(0,window);
			sf::Vector2f touch_position = window.mapPixelToCoords(touch_pixel);

			if (event.type == sf::Event::TouchBegan)
			{			       
					if (clock.s_clock.getGlobalBounds().contains(touch_position.x,touch_position.y))
					{
						++current_screen;						
						if (current_screen > 1) { current_screen = 0; }

						switch (current_screen)
						{
						case 0:clock.initialization_by_the_system_clock(); clock_v1.sound.stop(); break;
						case 1:clock_v1.initialization_by_the_system_clock(); clock.sound.stop(); break;
						}
					}
				
			}

			clock_v1.mouse_event(window,event);
			clock_v1.sensor_event(window,event);
			
			if (event.type == sf::Event::Closed)
				window.close();


		}
		window.clear();
		
		switch (current_screen)
		{
		case 0:window.draw(clock); break;
		case 1:window.draw(clock_v1); break;
		}
		window.display();
	}

	return 0;
}