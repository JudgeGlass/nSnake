#include <os.h>
#include <SDL/SDL_config.h>
#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "apple.h"
#include "player.h"
#include "menu.h"
#include "background.h"
#include "gameover.h"

SDL_Surface *screen;
SDL_Surface *background;
SDL_Surface *img_gameover;
SDL_Surface *img_menu;
SDL_Surface *img_player;
SDL_Surface *img_apple;

static Uint8 done;
static Uint8 b_gameover;
static Uint8 b_menu;
static Uint8 start;
static Uint8 last_x;
static Uint8 last_y;
static short dir;

typedef struct PlayerParts
{
    Uint8 x;
    Uint8 y;
    struct PlayerParts *next;
} parts_t;

parts_t *head;

parts_t* create_part(){
    parts_t* temp;
    temp = malloc(sizeof(parts_t));
    temp->next = NULL;
    return temp;
}

parts_t add_part(parts_t *head, Uint8 x, Uint8 y){
    parts_t *current = head;
    while(current->next != NULL){
        current = current->next;
    }

    current->next = malloc(sizeof(parts_t));
    current->next->x = x;
    current->next->y = y;
    current->next->next = NULL;
}

void init(void){
    SDL_Init(SDL_INIT_VIDEO);
    screen = SDL_SetVideoMode(320, 240, has_colors ? 16 : 8, SDL_SWSURFACE);
    done = SDL_FALSE;
}

void quit(void){
    SDL_FreeSurface(screen);
    SDL_FreeSurface(img_player);
    SDL_FreeSurface(img_apple);
    

    SDL_Quit();
    done = SDL_TRUE;
}

SDL_Surface* load_image(const char *filename){
    SDL_Surface *image = SDL_LoadBMP(filename);
    SDL_Surface *op_image = SDL_DisplayFormat(image);
    SDL_FreeSurface(image);
    image = NULL;
    return op_image;
}

Uint8 randomUint(Uint8 min, Uint8 max){
    return (rand() % (max - min + 1));
}

void draw_image(SDL_Surface* src, SDL_Surface* des, int x, int y){
    SDL_Rect rect_pos;
    rect_pos.x = x;
    rect_pos.y = y;
    SDL_BlitSurface(src, NULL, des, &rect_pos);
}

void gameover(){
    b_gameover = 1;
}

Uint8 hit_itself(parts_t *head, Uint8 offx, Uint8 offy){
    if(head->x > 19 || head->x < 0 || head->y > 14 || head->y < 0){
            return 1;
        }
    parts_t *temp = head;
    while(temp->next != NULL){
        if(head->x == temp->x - offx && head->y == temp->y - offy){
            return 1;
        }
        temp = temp->next;
    }

    return 0;
}

void move_player(){
    Uint8 add_x = 0;
    Uint8 add_y = 0;

    switch (dir)
    {
        case 1:
        case -1:
            add_x = dir;
            break;
        case 0:
            add_y = 1;
            break;
        case 2:
            add_y = -1;
    default:
        break;
    }

    last_x = head->x;
    last_y = head->y;
    head->x += add_x;
    head->y += add_y;

    if(head->x == apple_x && head->y == apple_y){
        if(head->next == NULL){
            add_part(head, last_x, last_y);
        }else{
            parts_t *part = head->next;
            int index = 0;
            while(part->next != NULL){
                index++;
                part = part->next;
            }
            add_part(head, part->x, part->y);
        }

        apple_x = randomUint(1, 20);
        apple_y = randomUint(1, 15);
    }

    Uint8 xx, yy;
    Uint8 xxx, yyy;
    Uint8 first = 1;

    parts_t *part = head->next;
    while(part != NULL){

        if(first){
            xx = part->x;
            yy = part->y;
            part->x = last_x;
            part->y = last_y;
            first = 0;
        }else{
            xxx = xx;
            yyy = yy;
            xx = part->x;
            yy = part->y;
            part->x = xxx;
            part->y = yyy;
        }
        
        part = part->next;
    }

    if(hit_itself(head, add_x, add_y)){
        gameover();
        return;
    }
}

Uint8 handle_key(SDLKey key){
    switch (key)
    {
        case SDLK_ESCAPE:
            quit();
            break;
        case SDLK_UP:
            dir = 2;
            break;
        case SDLK_DOWN:
            dir = 0;
            break;
        case SDLK_LEFT:
            dir = -1;
            break;
        case SDLK_RIGHT:
            dir = 1;
            break;
        case SDLK_r:
            if(b_gameover){
                b_gameover = 0;
            }
            break;
        case SDLK_s:
            b_menu = 0;
            start = 1;
            break;
        default:
            return 0;
    }

}


int main(){
    srand(time(0));
    apple_x = randomUint(1, 20);
    apple_y = 4;

    head = create_part();
    head->x = 4;
    head->y = 4;

    /*
         1 - Right
        -1 - Left
         0 - Down
         2 - Up
     */

    dir = 1;

    init();

    background = nSDL_LoadImage(image_background);
    img_gameover = nSDL_LoadImage(image_gameover);
    img_menu = nSDL_LoadImage(image_menu);
    img_player = nSDL_LoadImage(image_player);
    img_apple = nSDL_LoadImage(image_apple);

    start = 1;
    b_menu = 1;

    while(!done){
        if(b_menu){
            draw_image(img_menu, screen, 0, 0);
            SDL_Flip(screen);
            SDL_Delay(2000);
            b_menu = 0;
        }

        if(b_gameover){
            draw_image(img_gameover, screen, 12, 24);
            SDL_Flip(screen);
            SDL_Delay(2000);
            quit();
            b_gameover = 0;
        }

        if(start){
            draw_image(background, screen, 0, 0);
            draw_image(img_apple, screen, apple_x * 16, apple_y * 16);
            
            //printf("Player X: %d\tY: %d\n", head->x, head->y);
            parts_t *part;
            part = head;
            draw_image(img_player, screen, part->x * 16, part->y * 16);
            while(part->next != NULL){
                draw_image(img_player, screen, part->x * 16, part->y * 16);
                part = part->next;
            }
        }

        SDL_Event event;
        if(SDL_PollEvent(&event)){
            switch (event.type)
            {
            case SDL_KEYDOWN:
                handle_key(event.key.keysym.sym);
                break;
                
            default:
                break;
            }
        }

        if(start){
            move_player();
        }

        
        SDL_Flip(screen);
        SDL_Delay(100);
    }

    SDL_Delay(500);
    SDL_Quit();
    return 0;
}

