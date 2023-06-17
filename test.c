#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>


#define width 1697
#define height 930
#define FPS 60
#define CLOSED 881
#define PI 3.1415926

typedef struct _try_p
{
	double x1, x2, x3, y1, y2, y3;
}try_p;

ALLEGRO_DISPLAY* main_dy = NULL;
ALLEGRO_BITMAP* icon = NULL;
ALLEGRO_COLOR black, white, sky_blue;
ALLEGRO_EVENT_QUEUE* queue = NULL;
//ALLEGRO_EVENT_QUEUE * timer_queue = NULL;
ALLEGRO_EVENT event;//, timer
ALLEGRO_TIMER* game_time = NULL;
ALLEGRO_FONT* fps = NULL;
ALLEGRO_SAMPLE* start_bgm = NULL;
ALLEGRO_SAMPLE* start_press = NULL;
ALLEGRO_SAMPLE* gaming_bgm = NULL;
ALLEGRO_SAMPLE* jump_s = NULL;
ALLEGRO_SAMPLE* hit = NULL;

//ALLEGRO_THREAD* collision = NULL;


bool key_pressed[ALLEGRO_KEY_MAX];
double px = 400.0, py = 675.0;
int gforce = 800, count = 0, fram = 0;
int amount, start = 0, flor_run = 10, num;
double phy_t = 0;
double center[20][2] = {0};
try_p bg_cloud[20];
int obs_v = 0;
int score = 0;
int score_t = 0;
int check = 0;
int regen_t = 600, regen_t2 = 300;
double obs_p[3][4];


void game_init();
void draw_jump();
void reset_dy();
void destroy();
int jump();
void event_init();
void bg_obj_set();
void bg_cloud_gen();
void bg_cloud_run();
void bg_deaw_flor();
void bg_draw_obs();
void bg_obs_gen();
//void coll();
int dead_sc();
int event_process();
int game_run();



void main() {
	game_init();


	black = al_map_rgb(0, 0, 0);
	white = al_map_rgb(250, 250, 250);
	sky_blue = al_map_rgb(32, 167, 235);


	//reset_dy();
	//draw_jump();

	game_time = al_create_timer(1.0 / FPS);
	//timer_queue = al_create_event_queue();
	event_init();

	//starting screen 
	bool start = false;
	while (!start) {
		al_clear_to_color(white);
		al_draw_text(fps, black, width / 2, height / 2, ALLEGRO_ALIGN_CENTER, "Press Space To Start!!");
		al_flip_display();
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_KEY_UP && event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
			start = true;
		}
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			check = -1;
			break;
		}
	}

	al_start_timer(game_time);

	//generating back ground and obstical
	while (1) {
		al_wait_for_event(queue, &event);
		if (check == -1) {
			break;
		}
		if (regen_t / 60 == 10) {
			//printf("g%d\n", fram);
			bg_cloud_gen();
			regen_t = 0;
		}
		if (regen_t2 / 60 == 5) {
			bg_obs_gen();
			regen_t2 = 0;
		}
		//if (event.type == ALLEGRO_EVENT_TIMER) {
		regen_t++;
		regen_t2++;
		//}
		check = game_run();

		//check collision
		/*if (collision_ch() == CLOSED) {
			break;
		}*/

		for (int i = 0; i < num; i++) {
			if (((obs_p[i][0] > px - 35 && obs_p[i][0] < px + 35) ||
				(obs_p[i][2] > px - 35 && obs_p[i][2] < px + 35)) && obs_p[i][1] - py < 35) {
				check = 1;
			}
		}

		if (check == 1) {
			check = dead_sc();
		}

		//score count
		if (score_t++ / 60 == 1) {
			score++;
			score_t = 0;
		}
		reset_dy();
		/*fram++;   // fram checker;
		printf("%lf %lf\n", px, py);
		f (fram == 60)
			fram = 0*/

	}
	//al_set_thread_should_stop(collision);
	//al_join_thread(collision, NULL);
	destroy();
}

	void game_init() {
	if (!al_init()) {
		printf("Inition failed!!");
	}
	al_init_primitives_addon();
	al_init_image_addon();
	al_init_ttf_addon();
	al_init_font_addon();
	al_install_audio();
	al_init_acodec_addon();
	al_reserve_samples(5);

	icon = al_load_bitmap("icon.png");
	main_dy = al_create_display(width, height);
	al_set_window_position(main_dy, 100, 0);
	al_set_display_icon(main_dy, icon);
	al_set_window_title(main_dy, "Jumpy Guy");

	fps = al_load_font("pirulen.ttf", 50, 0);
	//collision = al_create_thread(collision_ch, fps);
	
	al_install_keyboard();
	queue = al_create_event_queue();

	start_bgm = al_load_sample("start_bgm.mp3");
	start_press = al_load_sample("start_press.mp3");
	gaming_bgm = al_load_sample("gaming_bgm.mp3");
	jump_s = al_load_sample("jump_s.mp3");
	hit = al_load_sample("hit.mp3");
}

void event_init() {
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(main_dy));
	al_register_event_source(queue, al_get_timer_event_source(game_time));
}

int game_run() {
	int re = 0;

	re = event_process();

	if (re == CLOSED) {
		return -1;
	}
	return re;
}

int event_process() {
	int ch = 0;
	//al_wait_for_event(queue, &event);
	
	if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
		return CLOSED;
	}
		
	if (event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
		key_pressed[event.keyboard.keycode] = true;
	}
	else if (event.type == ALLEGRO_EVENT_KEY_UP) {
		key_pressed[event.keyboard.keycode] = false;
	}

	if (key_pressed[ALLEGRO_KEY_SPACE] == true) {
		ch = jump();
		key_pressed[ALLEGRO_KEY_SPACE] = false;
	}
	return ch;
}

int jump() {
	int ch = 0;
	count = 0;
	//draw_jump();
	while (py <= 675.0) {
		al_wait_for_event(queue, &event);
		//al_drop_next_event(queue);
		if (event.type == ALLEGRO_EVENT_TIMER) {
			count++;
			phy_t = count / 60.0;
			py = py - 1 * ((165 * phy_t) - (gforce * phy_t * phy_t) / 2);
			if (py < 675.0) {
				reset_dy();
			}
			//printf("%lf %lf\n", px, py);
		}

		for (int i = 0; i < num; i++) {
			if (((obs_p[i][0] > px - 35 && obs_p[i][0] < px + 35) || 
				 (obs_p[i][2] > px - 35 && obs_p[i][2] < px + 35)) && obs_p[i][1] - py < 35 ) {
				return 1;
			}
		}
		/*check = collision_ch();

		if (check == CLOSED) {
			return CLOSED;
		}*/
		score_t++;
		regen_t++;
		regen_t2++;
		
	}
	/*if (py < 675.0) {
		key_pressed[ALLEGRO_KEY_SPACE] = true;
	}*/
	/*if (py >= 675.0) {
		
	}*/
	py = 675.0;
	count = 0;
	phy_t = 0.0;
	return ch;
}

void bg_obj_set() {
	for (int i = 0; i < amount; i++) {
		al_draw_filled_triangle(bg_cloud[i].x1, bg_cloud[i].y1,
								bg_cloud[i].x2, bg_cloud[i].y2,
								bg_cloud[i].x3, bg_cloud[i].y3,
								black);
	}
}

void reset_dy() {
	al_clear_to_color(white);
	al_draw_filled_rectangle(0, 0, width, height, sky_blue);//al_map_rgb(8, 196, 29)
	al_draw_filled_rectangle(0, height - 150, width, height, al_map_rgb(138, 119, 34));
	al_draw_filled_rectangle(0, 695, width, height - 130, al_map_rgb(75, 207, 38));
	bg_deaw_flor();
	al_draw_filled_circle(px, py, 50, al_map_rgb(255, 246, 24));
	al_draw_filled_circle(px + 35, py, 4, black);
	bg_draw_obs();
	bg_cloud_run();
	bg_obj_set();
	al_draw_textf(fps, black, width / 2, height / 2, ALLEGRO_ALIGN_CENTER, "score:%d", score);
	al_flip_display();
	//al_draw_filled_triangle(fp, 30, fp - 30, 60, fp + 30, 60, black);
}

void draw_jump() {
	/*reset_dy();
	
	al_flip_display();*/
}

void bg_cloud_run() {
	for (int i = 0; i < amount; i++) {
		bg_cloud[i].x1 -= 5;
		bg_cloud[i].x2 -= 5;
		bg_cloud[i].x3 -= 5;
	}
	bg_obj_set();
}

void bg_cloud_gen() {
	srand(time(NULL));
	amount = (abs(rand() % 16)) + 5;
	int space = 30;
	for (int i = 0; i < amount; i++) {
		center[i][0] = width + 30 + (space * (abs(rand()) % 41));
		center[i][1] = abs(rand()) % 301;
	}

	for (int i = 0; i < amount; i++) {
		bg_cloud[i].x1 = center[i][0] + 10 * cos(PI / 4);
		bg_cloud[i].y1 = center[i][1] + 10 * sin(PI / 4);
		bg_cloud[i].x2 = center[i][0] + 10 * cos((PI * 11) / 12);
		bg_cloud[i].y2 = center[i][1] + 10 * sin((PI * 11) / 12);
		bg_cloud[i].x3 = center[i][0] + 10 * cos((PI * 19) / 12);
		bg_cloud[i].y3 = center[i][1] + 10 * sin((PI * 19) / 12);
	}
}

void bg_obs_gen() {
	srand(time(NULL));
	num = (abs(rand() % 3)) + 1;
	for (int i = 0; i < num; i++) {
		obs_p[i][0] = width + i * ((abs(rand()) % 4) * 400);
		obs_p[i][1] = 580 + ((abs(rand()) % 2) * 73);
		obs_p[i][2] = obs_p[i][0] + 50;
		obs_p[i][3] = 725;
	}
}

void bg_deaw_flor() {
	al_draw_line(0, 700, width + 10, 700, black, 12);
	al_draw_line(0, height - 150, width + 10, height - 150, black, 5);
	for (int i = start; i < width; i += 30) {
		al_draw_line(i, 700, i + 40, height - 150, black, 5);
		al_draw_line(i + 40, height - 150, i + 40, height, black, 5);
	}
	if (flor_run % 10 == 0) {
		if (flor_run == 80) {
			start = 0;
			flor_run = 0;
		}
		else
			start -= 10;
	}	
}

void bg_draw_obs() {
	srand(time(NULL));
	for (int i = 0; i < num; i++) {
		al_draw_filled_rectangle(obs_p[i][0], obs_p[i][1],
								 obs_p[i][2], obs_p[i][3],
								 al_map_rgb(abs(rand()) % 251, abs(rand()) % 251, abs(rand()) % 251));
	}

	for (int i = 0; i < num; i++) {
		obs_p[i][0] -= 15;
		obs_p[i][2] -= 15;
	}

	/*obs_v += 15;
	if (obs_p[][2] - obs_v < 0) {
		obs_v = 0;
	}*/
}

int dead_sc() {
	//collision check
	al_draw_text(fps, black, width / 2, height / 2 - 50, ALLEGRO_ALIGN_CENTER, "GG");
	al_draw_text(fps, black, width / 2, height / 2 + 50, ALLEGRO_ALIGN_CENTER, "Press Space To Restart!!");
	al_flip_display();
	while (1) {
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_KEY_UP && event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
			px = 400.0;
			py = 675.0;
			check = 0;
			score = 0;
			score_t = 0;
			regen_t = 600;
			regen_t2 = 300;
			for (int i = 0; i < num; i++) {
				obs_p[i][0] = width;
				obs_p[i][1] = height;
				obs_p[i][2] = width;
				obs_p[i][3] = height;
			}
			break;
		}
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			return -1;
			break;
		}
	}
	return NULL;
}

//void coll() {
//	ALLEGRO_FONT* end = NULL;
//	end = al_load_font("pirulen.ttf", 50, 0);
//	
//	al_destroy_font(end);
//}

void destroy() {
	al_destroy_display(main_dy);
	al_destroy_bitmap(icon);
	al_destroy_event_queue(queue);
	//al_destroy_thread(collision);
	//al_destroy_event_queue(timer_queue);
	al_destroy_font(fps);
	al_destroy_sample(start_bgm);
	al_destroy_sample(start_press);
	al_destroy_sample(gaming_bgm);
	al_destroy_sample(jump_s);
	al_destroy_sample(hit);
}












/*int space = 30;
	srand(time(NULL));
	for (int i = 0; i < 20; i++) {
		center[i][0] = width + 30 + (space * (abs(rand()) % 41));
		center[i][1] = abs(rand()) % 500;
	}

	for (int i = 0; i < 20; i++) {
		bg_cloud[i].x1 = center[i][0] + 40 * cos(PI / 4);
		bg_cloud[i].y1 = center[i][1] + 40 * sin(PI / 4);
		bg_cloud[i].x2 = center[i][0] + 40 * cos((PI * 11) / 12);
		bg_cloud[i].y2 = center[i][1] + 40 * sin((PI * 11) / 12);
		bg_cloud[i].x3 = center[i][0] + 40 * cos((PI * 19) / 12);
		bg_cloud[i].y3 = center[i][1] + 40 * sin((PI * 19) / 12);
	}*/