#include "Amo.hpp"
#include "Core.hpp"
#include "Star.hpp"
#include "Enemy.hpp"
#include "Collection.hpp"

#include <ctime>
#include <iostream>

Core::Core(void)
{
	initscr();
	noecho();
	curs_set(false);
	nodelay(stdscr, true);

	_score = 0;
	_win = newwin(LINES, COLUMNS, 0, 0);
	_info = newwin(40, 20, 0, 180);
	
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_CYAN, COLOR_BLACK);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);

	_player = new Player(COLUMNS / 2, LINES - 2, 2);
	
	_steroids = new Collection();
	_bullets = new Collection();
	_enemies = new Collection();
}

Core::~Core()
{
	delwin(_win);
	endwin();
}

Core::Core(Game const &assign)
{
	*this = other;
}

Core &Core::operator =(Core const &assign)
{
	if (this != &assign)
	{
		_player = assign._player;
	}
	return *this;
}

bool Core::checkPenetration(Object *enemy)
{
	for (int i = 0; i < _bullets->getCount(); i++)
	{
		if (enemy->getCordX() == _bullets->getUnit(i)->getCordX() &&
			enemy->getCordY() == _bullets->getUnit(i)->getCordY())
		{
			_score++;
			return true;
		}
	}
	return false;
}

bool Core::checkCollision(Object *enemy)
{
	if (_player->getCordX() == enemy->getCordX() &&
		_player->getCordY() == enemy->getCordY())
	{
		_score++;
		return true;
	}
	return false;
}


void Core::moveEnemies()
{
	if ((float)(clock() - _enemyTime) / CLOCKS_PER_SEC > 0.1)
	{
		for (int i = 0; i < _enemies->getCount(); i++)
		{
			if (_enemies->getUnit(i) != NULL && _enemies->getUnit(i)->getCordY() > LINES -2)
				_enemies->getUnit(i)->setCordY(0);
			else if (_enemies->getUnit(i) != NULL && checkPenetration(_enemies->getUnit(i)) == true)
				_enemies->setNullUnit(i);
			else if (_enemies->getUnit(i) != NULL && checkCollision(_enemies->getUnit(i)) == true)
			{
				_player->setLives(_player->getLives() - 1);
				_enemies->setNullUnit(i);
			}
			else if (_enemies->getUnit(i) != NULL)
			{
				mvwaddstr(_win, _enemies->getUnit(i)->getCordY(), _enemies->getUnit(i)->getCordX(), "#");
				_enemies->getUnit(i)->moveD();
			}
		}
	}
	else
	{
		for (int i = 0; i < _enemies->getCount(); i++)
		{
			if (_enemies->getUnit(i) != NULL && _enemies->getUnit(i)->getY() > LINES - 2)
				_enemies->getUnit(i)->setY(0);
			else if (_enemies->getUnit(i) != NULL && checkPenetration(_enemies->getUnit(i)) == true)
				_enemies->setNullUnit(i);
			else if (_enemies->getUnit(i) != NULL && checkCollision(_enemies->getUnit(i)) == true)
			{
				_player->setLives(_player->getLives() - 1);
				_enemies->setNullUnit(i);
			}
			else if (_enemies->getUnit(i) != NULL)
				mvwaddstr(_win, _enemies->getUnit(i)->getY(), _enemies->getUnit(i)->getX(), "#");
		}
	}
	_enemyTime = clock();
	
}

void Core::printInfo()
{

	wattron(_info, COLOR_PAIR(2));
	mvwprintw(_info, 1, 6, "*GAME*", _player->getLives());
	mvwprintw(_info, 4, 2, "LIVES: %d", _player->getLives());

	mvwprintw(_info, 6, 2, "SCORE: %d", _score);

	mvwprintw(_info, 8, 2, "TIME: %ds", (time(NULL) - _startGame));

}

void Core::start()
{
	srand(clock());

	int key = -1;

	// RANDOM STEROIDS
	for (int i = 0; i < 50; i++)
	{
		_steroids->push(new Steroid(rand() % COLUMNS, rand() % LINES));
	}

	// RANDOM ENEMIES
	for (int i = 0; i < 10; i++)
	{
		int rX = 3 + (rand() % 150);
		int rY = 2 + (rand() % 5);
		_enemies->push(new SmallEnemy(rX, rY));

	}

	_lastTime = clock();
	_enemyTime = clock();
	_startGame = time(NULL);

	
	while (true)
	{
		if (_player->getLives() == 0 || _score == _enemies->getCount())
			break ;

		printInfo();
		if ((float)(clock() - _lastTime) / CLOCKS_PER_SEC < 0.1)
		{
			continue ;
		}
		_lastTime = clock();

		wclear(_win);
		wattron(_win, COLOR_PAIR(1));
		wattron(_info, COLOR_PAIR(1));
		box(_win, 0, 0);
		box(_info, 0, 0);
		wattroff(_win, COLOR_PAIR(1));
		wattroff(_info, COLOR_PAIR(1));


		for (int i = 0; i < _steroids->getCount(); i++)
		{
			if (_steroids->getUnit(i)->getY() > LINES - 2)
				_steroids->getUnit(i)->setY(0);
			else if (_steroids->getUnit(i)->getX() > 0 && _steroids->getUnit(i)->getX() < COLUMNS - 2)
				mvwaddstr(_win, _steroids->getUnit(i)->getY(), _steroids->getUnit(i)->getX(), ".");
			_steroids->getUnit(i)->moveDown();
		}

		// PLAYER
		wattron(_win, COLOR_PAIR(3));
		mvwaddstr(_win, _player->getY(), _player->getX(), "^");
		wattroff(_win, COLOR_PAIR(3));

		if ( key == 32 )
		{
			Bullet *newBullet = _player->shoot();
			_bullets->push(newBullet);
		}
		if ( key == 97 && _player->getX() > 3 )
			_player->moveLeft();
		else if ( key == 100 && _player->getX() < COLUMNS - 3 )
			_player->moveRight();
		
		

		// BULLETS
		for (int i = 0; i < _bullets->getCount(); i++)
		{
			if (_bullets->getUnit(i)->getY() > 0)
			{
				wattron(_win, COLOR_PAIR(4));
				mvwaddstr(_win, _bullets->getUnit(i)->getY(), _bullets->getUnit(i)->getX(), "|");
				wattroff(_win, COLOR_PAIR(3));
			}
			_bullets->getUnit(i)->moveUp();
		}

		// ENEMIES

		moveEnemies();
		
		// REFRESH
		wrefresh(_win);
		wrefresh(_info);

		key = getch();

		if (key == 27)
			break ;
	}
}
