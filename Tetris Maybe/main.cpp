#include <iostream>
#include <SDL.h>
#include <stdio.h>
#include <vector>

#define window_width 1000
#define window_height 800
#define pxl 20
#define posBoard 340
#define bw 10
#define bh 21
#define delay 100 //should be slowed down, run time is limited due to memory issues (I think)
#define fps 60

using namespace std;

SDL_Window* window=NULL;
SDL_Surface* ScreenSurface=NULL;
SDL_Surface* bckrnd=NULL;

int grid [bw][bh]; //10x21 grid, which is the board
vector <int> posx; //x coordinate of a point on the board
vector <int> posy; //y coordinate of a point on the board
vector <int> htr; //how to rotate, depending on the type of tetromino
vector <int> numR; //rotation number, 4 different orientation for each tetromino
int PieceCounter=0; //number of pieces created
int tetrominos [7][5][2]
//type - block - 0=x coordinate, 1=y coordinate
//straight
{
    tetrominos[0][0][0]=0,//there is an empty zero block otherwise a piece of the tetromino would be stored
                          //in the array as a 0, which signifies an empty space (which it is not empty)
    tetrominos[0][0][1]=0,
    tetrominos[0][1][0]=3,
    tetrominos[0][1][1]=1,
    tetrominos[0][2][0]=4,
    tetrominos[0][2][1]=1,
    tetrominos[0][3][0]=5,
    tetrominos[0][3][1]=1,
    tetrominos[0][4][0]=6,
    tetrominos[0][4][1]=1,
    //L
    tetrominos[1][0][0]=0,
    tetrominos[1][0][1]=0,
    tetrominos[1][1][0]=5,
    tetrominos[1][1][1]=3,
    tetrominos[1][2][0]=5,
    tetrominos[1][2][1]=1,
    tetrominos[1][3][0]=5,
    tetrominos[1][3][1]=2,
    tetrominos[1][4][0]=4,
    tetrominos[1][4][1]=1,
    //J
    tetrominos[2][0][0]=0,
    tetrominos[2][0][1]=0,
    tetrominos[2][1][0]=5,
    tetrominos[2][1][1]=1,
    tetrominos[2][2][0]=4,
    tetrominos[2][3][1]=1,
    tetrominos[2][3][0]=4,
    tetrominos[2][3][1]=2,
    tetrominos[2][4][0]=4,
    tetrominos[2][4][1]=3,
    //Z
    tetrominos[3][0][0]=0,
    tetrominos[3][0][1]=0,
    tetrominos[3][1][0]=3,
    tetrominos[3][1][1]=1,
    tetrominos[3][2][0]=4,
    tetrominos[3][2][1]=1,
    tetrominos[3][3][0]=4,
    tetrominos[3][3][1]=2,
    tetrominos[3][4][0]=5,
    tetrominos[3][4][1]=2,
    //S
    tetrominos[4][0][0]=0,
    tetrominos[4][0][1]=0,
    tetrominos[4][1][0]=5,
    tetrominos[4][1][1]=1,
    tetrominos[4][2][0]=4,
    tetrominos[4][2][1]=1,
    tetrominos[4][3][0]=4,
    tetrominos[4][3][1]=2,
    tetrominos[4][4][0]=3,
    tetrominos[4][4][1]=2,
    //T
    tetrominos[5][0][0]=0,
    tetrominos[5][0][1]=0,
    tetrominos[5][1][0]=3,
    tetrominos[5][1][1]=1,
    tetrominos[5][2][0]=4,
    tetrominos[5][2][1]=1,
    tetrominos[5][3][0]=5,
    tetrominos[5][3][1]=1,
    tetrominos[5][4][0]=4,
    tetrominos[5][4][1]=2,
    //Square
    tetrominos[6][0][0]=0,
    tetrominos[6][0][1]=0,
    tetrominos[6][1][0]=5,
    tetrominos[6][1][1]=1,
    tetrominos[6][2][0]=4,
    tetrominos[6][2][1]=1,
    tetrominos[6][3][0]=4,
    tetrominos[6][3][1]=2,
    tetrominos[6][4][0]=5,
    tetrominos[6][4][1]=2,
};
void cap_framerate( Uint32 starting_tick)
{
    if ((1000/fps) > SDL_GetTicks() - starting_tick)
    {
        SDL_Delay(1000/fps - (SDL_GetTicks() - starting_tick));
    }
}
bool init()
{
    //Initialization flag
    bool success=true;

    //Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO)<0)
    {
        cout<<SDL_GetError()<<endl;
        success=false;
    }
    else
    {
        window=SDL_CreateWindow
               ("Game",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,window_width,window_height,SDL_WINDOW_SHOWN);
        if(window==NULL)
        {
            cout<<SDL_GetError()<<endl;
            success=false;
        }
        else
        {
            ScreenSurface=SDL_GetWindowSurface(window);
        }
    }

    return success;
}
bool loadMedia()
{
    bool success=true;


    bckrnd=SDL_LoadBMP("Screen.bmp");
    if(bckrnd==NULL)
    {
        cout<<SDL_GetError()<<endl;
        success=false;
    }

    return success;
}
class sprite //class to create visual block
{
protected:
    SDL_Surface *image;
    SDL_Rect block;

public:
    sprite(Uint32 colour, int x, int y, int w=20, int h=20)
    {
        image=SDL_CreateRGBSurface( 0, w, h, 32, 0, 0, 0, 0 );
        SDL_FillRect(image,NULL,colour);
        block=image->clip_rect;
        block.x=x;
        block.y=y;
    }
    void draw (SDL_Surface *destination)
    {
        SDL_BlitSurface(image,NULL,destination,&block);
    }
    SDL_Surface* get_image() const
    {
        return image;
    }
    bool operator==(const sprite &other)const
    {
        return (image==other.image );
    }
    void set_image(const char filename[]=NULL )
    {
        if (filename!=NULL)
        {
            SDL_Surface *loaded_image=NULL;

            loaded_image=SDL_LoadBMP( filename );

            if (loaded_image !=NULL)
            {
                image=loaded_image;
                block=image->clip_rect;
            }
            if (loaded_image==NULL)
            {
                cout<<SDL_GetError()<<endl;
            }
        }
    }
};
class Board
{
public:
    bool VcanMove (int x, int y)
    {
        if ((grid[x][y+1]==7)||((y+1)>20)) //if it would collide with a pre-existing block or go past boundaries
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    bool HRcanMove (int x, int y)
    {
        if ((grid[x+1][y]==7)||((x+1)>=10)) //if it would collide with a pre-existing block or go past boundaries
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    bool HLcanMove (int x, int y)
    {
        if ((grid[x-1][y]==7)||((x==0))) //if it would collide with a pre-existing block or go past boundaries
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    bool CanRotate ()
    //based on what type of block and it's current rotation, would moving to the next rotation cause the piece to collide
    //with a pre-existing block or go past the boundaries
    {
        if (htr.at(PieceCounter-1)==0)
        {
            if (numR.at(PieceCounter-1)==0)
            {
                if (grid[posx.at((4*PieceCounter)-4)+1][posy.at((4*PieceCounter)-4)-1]!=0
                        || (posx.at((4*PieceCounter)-4)+1)>9 || (posy.at((4*PieceCounter)-4)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)-1][posy.at((4*PieceCounter)-2)+1]!=0
                        || (posx.at((4*PieceCounter)-2)-1)<0 || (posy.at((4*PieceCounter)-2)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)-2][posy.at((4*PieceCounter)-1)+2]!=0
                        || (posx.at((4*PieceCounter)-1)-2)<0 || (posy.at((4*PieceCounter)-1)+2)>20)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==1)
            {
                if (grid[posx.at((4*PieceCounter)-4)+1][posy.at((4*PieceCounter)-4)+1]!=0
                        || (posx.at((4*PieceCounter)-4)+1)>9 || (posy.at((4*PieceCounter)-4)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)-1][posy.at((4*PieceCounter)-2)-1]!=0
                        || (posx.at((4*PieceCounter)-2)-1)<0 || (posy.at((4*PieceCounter)-2)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)-2][posy.at((4*PieceCounter)-1)-2]!=0
                        || (posx.at((4*PieceCounter)-1)-2)<0 || (posy.at((4*PieceCounter)-1)-2)<=0)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==2)
            {
                if (grid[posx.at((4*PieceCounter)-4)-1][posy.at((4*PieceCounter)-4)+1]!=0
                        || (posx.at((4*PieceCounter)-4)-1)<0 || (posy.at((4*PieceCounter)-4)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)+1][posy.at((4*PieceCounter)-2)-1]!=0
                        || (posx.at((4*PieceCounter)-2)+1)>9 || (posy.at((4*PieceCounter)-2)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)+2][posy.at((4*PieceCounter)-1)-2]!=0
                        || (posx.at((4*PieceCounter)-1)+2)>9 || (posy.at((4*PieceCounter)-1)-2)<=0)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==3)
            {
                if (grid[posx.at((4*PieceCounter)-4)-1][posy.at((4*PieceCounter)-4)-1]!=0
                        || (posx.at((4*PieceCounter)-4)-1)<0 || (posy.at((4*PieceCounter)-4)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)+1][posy.at((4*PieceCounter)-2)+1]!=0
                        || (posx.at((4*PieceCounter)-2)+1)>9 || (posy.at((4*PieceCounter)-2)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)+2][posy.at((4*PieceCounter)-1)+2]!=0
                        || (posx.at((4*PieceCounter)-1)+2)>9 || (posy.at((4*PieceCounter)-1)+2)>20)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }
        else if (htr.at(PieceCounter-1)==1)
        {
            if (numR.at(PieceCounter-1)==0)
            {
                if (grid[posx.at((4*PieceCounter)-4)-2][posy.at((4*PieceCounter)-4)-2]==7
                        || (posx.at((4*PieceCounter)-4)-2)<0 || (posy.at((4*PieceCounter)-4)-2)<=0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)-1][posy.at((4*PieceCounter)-2)-1]==7
                        || (posx.at((4*PieceCounter)-2)-1)<0 || (posy.at((4*PieceCounter)-2)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)+1][posy.at((4*PieceCounter)-1)-1]==7
                        || (posx.at((4*PieceCounter)-1)+1)>9 || (posy.at((4*PieceCounter)-1)-1)==0)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==1)
            {
                if (grid[posx.at((4*PieceCounter)-4)+2][posy.at((4*PieceCounter)-4)-2]==7
                        || (posx.at((4*PieceCounter)-4)+2)>9 || (posy.at((4*PieceCounter)-4)-2)<=0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)+1][posy.at((4*PieceCounter)-2)-1]==7
                        || (posx.at((4*PieceCounter)-2)+1)>9 || (posy.at((4*PieceCounter)-2)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)+1][posy.at((4*PieceCounter)-1)+1]==7
                        || (posx.at((4*PieceCounter)-1)+1)>9 || (posy.at((4*PieceCounter)-1)+1)>20)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==2)
            {
                if (grid[posx.at((4*PieceCounter)-4)+2][posy.at((4*PieceCounter)-4)+2]==7
                        || (posx.at((4*PieceCounter)-4)+2)>9 || (posy.at((4*PieceCounter)-4)+2)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)+1][posy.at((4*PieceCounter)-2)+1]==7
                        || (posx.at((4*PieceCounter)-2)+1)>9 || (posy.at((4*PieceCounter)-2)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)-1][posy.at((4*PieceCounter)-1)+1]==7
                        || (posx.at((4*PieceCounter)-1)-1)<0 || (posy.at((4*PieceCounter)-1)+1)>20)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==3)
            {
                if (grid[posx.at((4*PieceCounter)-4)-2][posy.at((4*PieceCounter)-4)+2]==7
                        || (posx.at((4*PieceCounter)-4)-2)<0 || (posy.at((4*PieceCounter)-4)+2)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)-1][posy.at((4*PieceCounter)-2)+1]==7
                        || (posx.at((4*PieceCounter)-2)-1)<0 || (posy.at((4*PieceCounter)-2)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)-1][posy.at((4*PieceCounter)-1)-1]==7
                        || (posx.at((4*PieceCounter)-1)-1)<0 || (posy.at((4*PieceCounter)-1)-1)==0)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }
        else if (htr.at(PieceCounter-1)==2)
        {
            if (numR.at(PieceCounter-1)==0)
            {
                if (grid[posx.at((4*PieceCounter)-4)-1][posy.at((4*PieceCounter)-4)+1]==7
                        || (posx.at((4*PieceCounter)-4)-1)<0 || (posy.at((4*PieceCounter)-4)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)-1][posy.at((4*PieceCounter)-2)-1]==7
                        || (posx.at((4*PieceCounter)-2)-1)<0 || (posy.at((4*PieceCounter)-2)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)-2][posy.at((4*PieceCounter)-1)-2]==7
                        || (posx.at((4*PieceCounter)-1)-2)<0 || (posy.at((4*PieceCounter)-1)-2)<=0)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==1)
            {
                if (grid[posx.at((4*PieceCounter)-4)-1][posy.at((4*PieceCounter)-4)-1]==7
                        || (posx.at((4*PieceCounter)-4)-1)<0 || (posy.at((4*PieceCounter)-4)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)+1][posy.at((4*PieceCounter)-2)-1]==7
                        || (posx.at((4*PieceCounter)-2)+1)>9 || (posy.at((4*PieceCounter)-2)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)+2][posy.at((4*PieceCounter)-1)-2]==7
                        || (posx.at((4*PieceCounter)-1)+2)>9 || (posy.at((4*PieceCounter)-1)-2)<=0)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==2)
            {
                if (grid[posx.at((4*PieceCounter)-4)+1][posy.at((4*PieceCounter)-4)-1]==7
                        || (posx.at((4*PieceCounter)-4)+1)>9 || (posy.at((4*PieceCounter)-4)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)+1][posy.at((4*PieceCounter)-2)+1]==7
                        || (posx.at((4*PieceCounter)-2)+1)>9 || (posy.at((4*PieceCounter)-2)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)+2][posy.at((4*PieceCounter)-1)+2]==7
                        || (posx.at((4*PieceCounter)-1)+2)>9 || (posy.at((4*PieceCounter)-1)+2)>20)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==3)
            {
                if (grid[posx.at((4*PieceCounter)-4)+1][posy.at((4*PieceCounter)-4)+1]==7
                        || (posx.at((4*PieceCounter)-4)+1)>9 || (posy.at((4*PieceCounter)-4)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)-1][posy.at((4*PieceCounter)-2)+1]==7
                        || (posx.at((4*PieceCounter)-2)-1)<0 || (posy.at((4*PieceCounter)-2)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)-2][posy.at((4*PieceCounter)-1)+2]==7
                        || (posx.at((4*PieceCounter)-1)-2)<0 || (posy.at((4*PieceCounter)-1)+2)>20)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }
        else if (htr.at(PieceCounter-1)==3)
        {
            if (numR.at(PieceCounter-1)==0)
            {
                if (grid[posx.at((4*PieceCounter)-4)+1][posy.at((4*PieceCounter)-4)-1]==7
                        || (posx.at((4*PieceCounter)-4)+1)>9 || (posy.at((4*PieceCounter)-4)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)-1][posy.at((4*PieceCounter)-2)-1]==7
                        || (posx.at((4*PieceCounter)-2)-1)<0 || (posy.at((4*PieceCounter)-2)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)-2][posy.at((4*PieceCounter)-1)]!=0
                        || (posx.at((4*PieceCounter)-1)-2)<0)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==1)
            {
                if (grid[posx.at((4*PieceCounter)-4)+1][posy.at((4*PieceCounter)-4)+1]==7
                        || (posx.at((4*PieceCounter)-4)+1)>9 || (posy.at((4*PieceCounter)-4)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)+1][posy.at((4*PieceCounter)-2)-1]==7
                        || (posx.at((4*PieceCounter)-2)+1)>9 || (posy.at((4*PieceCounter)-2)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)-2]==7
                        || (posx.at((4*PieceCounter)-1)-1)<0 || (posy.at((4*PieceCounter)-1)-2)<=0)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==2)
            {
                if (grid[posx.at((4*PieceCounter)-4)-1][posy.at((4*PieceCounter)-4)+1]==7
                        || (posx.at((4*PieceCounter)-4)-1)<0 || (posy.at((4*PieceCounter)-4)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)+1][posy.at((4*PieceCounter)-2)+1]==7
                        || (posx.at((4*PieceCounter)-2)+1)>9 || (posy.at((4*PieceCounter)-2)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)+2][posy.at((4*PieceCounter)-1)]==7
                        || (posx.at((4*PieceCounter)-1)+2)>9)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==3)
            {
                if (grid[posx.at((4*PieceCounter)-4)-1][posy.at((4*PieceCounter)-4)-1]==7
                        || (posx.at((4*PieceCounter)-4)-1)<0 || (posy.at((4*PieceCounter)-4)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)-1][posy.at((4*PieceCounter)-2)+1]==7
                        || (posx.at((4*PieceCounter)-2)-1)<0 || (posy.at((4*PieceCounter)-2)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)+2]==7
                        || (posy.at((4*PieceCounter)-1)+2)>20)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }
        else if (htr.at(PieceCounter-1)==4)
        {
            if (numR.at(PieceCounter-1)==0)
            {
                if (grid[posx.at((4*PieceCounter)-4)-1][posy.at((4*PieceCounter)-4)+1]==7
                        || (posx.at((4*PieceCounter)-4)-1)<20 || (posy.at((4*PieceCounter)-4)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)-1][posy.at((4*PieceCounter)-2)-1]==7
                        || (posx.at((4*PieceCounter)-2)-1)<0 || (posy.at((4*PieceCounter)-2)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)-2]==7
                        ||(posy.at((4*PieceCounter)-1)-2)<=0)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==1)
            {
                if (grid[posx.at((4*PieceCounter)-4)-1][posy.at((4*PieceCounter)-4)-1]==7
                        || (posx.at((4*PieceCounter)-4)-1)<0 || (posy.at((4*PieceCounter)-4)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)+1][posy.at((4*PieceCounter)-2)-1]==7
                        || (posx.at((4*PieceCounter)-2)+1)>9 || (posy.at((4*PieceCounter)-2)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)+2][posy.at((4*PieceCounter)-1)]==7
                        || (posx.at((4*PieceCounter)-1)+2)>9)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==2)
            {
                if (grid[posx.at((4*PieceCounter)-4)+1][posy.at((4*PieceCounter)-4)-1]==7
                        || (posx.at((4*PieceCounter)-4)+1)>9 || (posy.at((4*PieceCounter)-4)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)+1][posy.at((4*PieceCounter)-2)+1]==7
                        || (posx.at((4*PieceCounter)-2)+1)>9 || (posy.at((4*PieceCounter)-2)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)+2]==7
                        ||(posy.at((4*PieceCounter)-1)+2)>20)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==3)
            {
                if (grid[posx.at((4*PieceCounter)-4)+1][posy.at((4*PieceCounter)-4)+1]==7
                        || (posx.at((4*PieceCounter)-4)+1)>9 || (posy.at((4*PieceCounter)-4)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)-1][posy.at((4*PieceCounter)-2)+1]==7
                        || (posx.at((4*PieceCounter)-2)-1)<0 || (posy.at((4*PieceCounter)-2)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)-2][posy.at((4*PieceCounter)-1)]==7
                        || (posx.at((4*PieceCounter)-1)-2)<0)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }
        else if (htr.at(PieceCounter-1)==5)
        {
            if (numR.at(PieceCounter-1)==0)
            {
                if (grid[posx.at((4*PieceCounter)-4)+1][posy.at((4*PieceCounter)-4)-1]==7
                        || (posx.at((4*PieceCounter)-4)+1)>9 || (posy.at((4*PieceCounter)-4)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)-1][posy.at((4*PieceCounter)-2)+1]==7
                        || (posx.at((4*PieceCounter)-2)-1)<0 || (posy.at((4*PieceCounter)-2)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)-1][posy.at((4*PieceCounter)-1)-1]==7
                        || (posx.at((4*PieceCounter)-1)-1)<0 || (posy.at((4*PieceCounter)-1)-1)==0)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==1)
            {
                if (grid[posx.at((4*PieceCounter)-4)+1][posy.at((4*PieceCounter)-4)+1]==7
                        || (posx.at((4*PieceCounter)-4)+1)>9 || (posy.at((4*PieceCounter)-4)+1)>20)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)-1][posy.at((4*PieceCounter)-2)-1]==7
                        || (posx.at((4*PieceCounter)-2)-1)<0 || (posy.at((4*PieceCounter)-2)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)+1][posy.at((4*PieceCounter)-1)-1]==7
                        || (posx.at((4*PieceCounter)-1)+1)>9 || (posy.at((4*PieceCounter)-1)-1)==0)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==2)
            {
                if (grid[posx.at((4*PieceCounter)-4)-1][posy.at((4*PieceCounter)-4)-1]==7
                        || (posx.at((4*PieceCounter)-4)-1)<0 || (posy.at((4*PieceCounter)-4)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)+1][posy.at((4*PieceCounter)-2)-1]==7
                        || (posx.at((4*PieceCounter)-2)+1)>9 || (posy.at((4*PieceCounter)-2)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)+1][posy.at((4*PieceCounter)-1)+1]==7
                        || (posx.at((4*PieceCounter)-1)+1)>9 || (posy.at((4*PieceCounter)-1)+1)>20)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if (numR.at(PieceCounter-1)==3)
            {
                if (grid[posx.at((4*PieceCounter)-4)-1][posy.at((4*PieceCounter)-4)-1]==7
                        || (posx.at((4*PieceCounter)-4)-1)<0 || (posy.at((4*PieceCounter)-4)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-2)+1][posy.at((4*PieceCounter)-2)+1]==7
                        || (posx.at((4*PieceCounter)-2)+1)>9 || (posy.at((4*PieceCounter)-4)-1)==0)
                {
                    return false;
                }
                if (grid[posx.at((4*PieceCounter)-1)-1][posy.at((4*PieceCounter)-1)+1]==7
                        || (posx.at((4*PieceCounter)-1)-1)<0 || (posy.at((4*PieceCounter)-1)+1)>20)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }
    }
    void ClearLine ()
    {
        int numCleared; //after a specific number of lines cleared, increase speed
        for (int i=bh-1; i>0; i--)
        {
            for (int a=0; a<bw; a++)
            {
                if (grid[a][i]==0) //check every row, if any element of a row is 0, move on to next row
                {
                    break;
                }
                if (a==9) //if all elements of a row are not zero
                {
                    for (int b=i; b>0; b--)
                    {
                        for (int c=0; c<bw; c++)
                        {
                            grid[c][b]=grid[c][b-1]; //have a line take on the elements of the above line
                        }
                    }
                    i=bh-1; //restart loop
                    a=0;    //restart loop
                    numCleared++;
                }
            }
        }
    }
    bool Game_Over ()
    {
        for (int i=0; i<bw; i++)
        {
            if (grid[i][0]!=0) //if a block is in the top row, the game ends
            {
                return true;
            }
        }
        return false;
    }
    void createGrid()
    {
        for (int a=0; a<bw; a++)
        {
            for (int b=0; b<bh; b++)
            {
                grid[a][b]=0; //set the whole grid to 00
            }
        }
        //grid[5][6]=4;
    }
};
class Game
{
public:
    void CreatePiece()
    {
        for (int a=0; a<bw; a++)
        {
            for (int b=0; b<bh; b++)
            {
                if (grid[a][b]!=0)
                {
                    grid[a][b]=7; //set previous piece that has stopped moving to turn white
                }
            }
        }
        int piecetype=rand()% 7; //one of 7 pieces
        //int piecetype=0;
        for (int a=1; a<5; a++)
        {
            {
                (grid[(tetrominos[piecetype][a][0])][(tetrominos[piecetype][a][1])])=a;
                posx.push_back(tetrominos[piecetype][a][0]); //x position of one of 4 blocks of tetromino
                posy.push_back(tetrominos[piecetype][a][1]);//y position of one of 4 blocks of tetromino
            }
        }
        PieceCounter++; //increase number of pieces created
        htr.push_back(piecetype); //the type of block that was created
        numR.push_back(0); //starting at rotation, or orientation, 0
        //vectors for posx, posy, htr, and numR will all have the sum number for one piece
    }
    void RotatePiece()
    {
        //based on the type of piece and it's current rotation, move each piece of the tetromino to a different location on the board relative
        //to a constant block (block 2) so that the piece is rotated
        if (htr.at(PieceCounter-1)==0)
        {
            if (numR.at(PieceCounter-1)==0)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)+1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)-1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)-1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)+1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)-2;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)+2;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==1)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)+1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)+1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)-1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)-1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)-2;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)-2;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==2)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)-1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)+1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)+1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)-1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)+2;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)-2;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==3)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)-1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)-1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)+1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)+1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)+2;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)+2;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
        }
        if (htr.at(PieceCounter-1)==1)
        {
            if (numR.at(PieceCounter-1)==0)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)-2;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)-2;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)-1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)-1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)+1;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)-1;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==1)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)+2;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)-2;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)+1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)-1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)+1;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)+1;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==2)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)+2;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)+2;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)+1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)+1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)-1;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)+1;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==3)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)-2;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)+2;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)-1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)+1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)-1;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)-1;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
        }
        if (htr.at(PieceCounter-1)==2)
        {
            if (numR.at(PieceCounter-1)==0)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)-1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)+1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)-1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)-1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)-2;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)-2;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==1)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)-1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)-1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)+1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)-1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)+2;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)-2;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==2)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)+1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)-1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)+1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)+1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)+2;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)+2;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==3)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)+1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)+1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)-1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)+1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)-2;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)+2;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
        }
        if (htr.at(PieceCounter-1)==3)
        {
            if (numR.at(PieceCounter-1)==0)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)+1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)-1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)-1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)-1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)-2;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1);
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==1)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)+1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)+1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)+1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)-1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1);
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)-2;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==2)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)-1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)+1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)+1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)+1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)+2;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1);
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==3)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)-1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)-1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)-1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)+1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1);
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)+2;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
        }
        if (htr.at(PieceCounter-1)==4)
        {
            if (numR.at(PieceCounter-1)==0)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)-1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)+1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)-1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)-1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1);
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)-2;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==1)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)-1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)-1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)+1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)-1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)+2;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1);
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==2)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)+1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)-1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)+1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)+1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1);
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)+2;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==3)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)+1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)+1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)-1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)+1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)-2;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1);
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
        }
        if (htr.at(PieceCounter-1)==5)
        {
            if (numR.at(PieceCounter-1)==0)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)+1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)-1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)-1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)+1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)-1;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)-1;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==1)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)+1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)+1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)-1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)-1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)+1;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)-1;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==2)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)-1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)+1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)+1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)-1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)+1;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)+1;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
            else if (numR.at(PieceCounter-1)==3)
            {
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=0;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=0;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=0;
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4)-1;
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4)-1;
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2)+1;
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2)+1;
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1)-1;
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1)+1;
                grid[posx.at((4*PieceCounter)-4)][posy.at((4*PieceCounter)-4)]=1;
                grid[posx.at((4*PieceCounter)-2)][posy.at((4*PieceCounter)-2)]=3;
                grid[posx.at((4*PieceCounter)-1)][posy.at((4*PieceCounter)-1)]=4;
            }
        }
        /*if (htr.at(PieceCounter-1)==6)
        {
            if (numR.at(PieceCounter-1)==0)
            {
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4);
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4);
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2);
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2);
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1);
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1);
            }
            else if (numR.at(PieceCounter-1)==1)
            {
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4);
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4);
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2);
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2);
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1);
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1);
            }
            else if (numR.at(PieceCounter-1)==2)
            {
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4);
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4);
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2);
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2);
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1);
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1);
            }
            else if (numR.at(PieceCounter-1)==3)
            {
                posx.at((4*PieceCounter)-4)=posx.at((4*PieceCounter)-4);
                posy.at((4*PieceCounter)-4)=posy.at((4*PieceCounter)-4);
                posx.at((4*PieceCounter)-2)=posx.at((4*PieceCounter)-2);
                posy.at((4*PieceCounter)-2)=posy.at((4*PieceCounter)-2);
                posx.at((4*PieceCounter)-1)=posx.at((4*PieceCounter)-1);
                posy.at((4*PieceCounter)-1)=posy.at((4*PieceCounter)-1);
            }
        }*/
        if (numR.at(PieceCounter-1)<3)
        {
            numR.at(PieceCounter-1)++;
        }
        else if (numR.at(PieceCounter-1)=3)
        {
            numR.at(PieceCounter-1)=0;
        }
    }
};

int main(int argc, char *argv[])
{
    Uint32 starting_tick;
    if(!init())
    {
        cout<<SDL_GetError()<<endl;
    }
    else
    {
        if( !loadMedia() )
        {
            cout<<SDL_GetError()<<endl;
        }
        else
        {
            SDL_BlitSurface( bckrnd, NULL, ScreenSurface, NULL );
            SDL_UpdateWindowSurface( window );
        }
    }
    Board xBoard;
    Game xGame;
    xBoard.createGrid();
    Uint32 black = SDL_MapRGB( ScreenSurface->format, 0, 0, 0);
    Uint32 white = SDL_MapRGB( ScreenSurface->format, 255, 255, 255);
    Uint32 red = SDL_MapRGB( ScreenSurface->format, 255, 0, 0);
    Uint32 green = SDL_MapRGB( ScreenSurface->format, 0, 128, 0);
    Uint32 blue = SDL_MapRGB( ScreenSurface->format, 0, 0, 255);
    Uint32 yellow = SDL_MapRGB( ScreenSurface->format, 255, 255, 0);
    xGame.CreatePiece(); //first piece
    SDL_Event event;
    bool running = true;
    starting_tick = SDL_GetTicks();
    while (running)
    {
        if (event.type == SDL_QUIT)
        {
            running = false;
            break;
        }
        for (int i=0; i<bw; i++)
        {
            for (int j=0; j<bh; j++)
            {
                if (grid[i][j]==0) //if not occupied, black
                {
                    sprite posFree (black, 400+(pxl*i), 160+(pxl*j), pxl, pxl);
                    posFree.draw(ScreenSurface);
                }
                //next 4 are the different colours used for the 4 different blocks of the tetromino
                else if ((grid[i][j]==1))
                {
                    sprite pos1 (red, 400+(pxl*i), 160+(pxl*j), pxl, pxl);
                    pos1.draw(ScreenSurface);
                }
                else if ((grid[i][j]==2))
                {
                    sprite pos2 (green, 400+(pxl*i), 160+(pxl*j), pxl, pxl);
                    pos2.draw(ScreenSurface);
                }
                else if ((grid[i][j]==3))
                {
                    sprite pos3 (blue, 400+(pxl*i), 160+(pxl*j), pxl, pxl);
                    pos3.draw(ScreenSurface);
                }
                else if ((grid[i][j]==4))
                {
                    sprite pos4 (yellow, 400+(pxl*i), 160+(pxl*j), pxl, pxl);
                    pos4.draw(ScreenSurface);
                }
                //if the element is a previous piece that was not able to move further and was stored
                else if ((grid[i][j]==7))
                {
                    sprite pos7 (white, 400+(pxl*i), 160+(pxl*j), pxl, pxl);
                    pos7.draw(ScreenSurface);
                }
            }
        }
        if(event.type==SDL_KEYDOWN)
        {
            switch( event.key.keysym.sym )
            {
            case SDLK_ESCAPE: //stop game when escape key is pressed
            {
                running=false;
                break;
            }
            }
        }
        while (SDL_PollEvent(&event))
        {
            if ((event.type==SDL_KEYDOWN) && (event.key.repeat==0))
            {
                switch(event.key.keysym.sym)
                {
                case SDLK_UP: //up key
                {
                    bool canMove;
                    canMove = xBoard.CanRotate(); //check if no collision if rotated
                    if (canMove==true)
                    {
                        xGame.RotatePiece(); //rotate piece if no collision
                    }
                    break;
                }
                case SDLK_RIGHT:
                {
                    bool canMove;
                    for (int i=4; i>0; i--)
                    {
                        //check each individual block to see if it collides with anything
                        canMove=xBoard.HRcanMove(posx[(4*PieceCounter)-i],posy[(4*PieceCounter)-i]);
                        if (canMove==false)
                        {
                            break; //if any of the blocks collide, stop checking
                        }
                    }
                    if (canMove==true) //if none collide
                    {
                        vector <int> tempx;
                        for (int i=4; i>0; i--)
                        {
                            posx[(4*PieceCounter)-i]++;
                            tempx.push_back(grid[(posx[(4*PieceCounter)-i])-1][((posy[(4*PieceCounter)-i]))]); //4 new coordinates of each block
                            grid[(posx[(4*PieceCounter)-i]-1)][((posy[(4*PieceCounter)-i]))]=0; //clear previous coordinate
                        }
                        for (int i=4; i>0; i--)
                        {
                            grid[(posx[(4*PieceCounter)-i])][(posy[(4*PieceCounter)-i])]=tempx[4-i]; //set the coordinate to the block
                        }
                    }
                    break;
                }
                case SDLK_LEFT:
                {
                    bool canMove;
                    for (int i=1; i<=4; i++)
                    {
                        canMove=xBoard.HLcanMove(posx[(4*PieceCounter)-i],posy[(4*PieceCounter)-i]);
                        if (canMove==false)
                        {
                            break;
                        }
                    }
                    if (canMove==true)
                    {
                        vector <int> temp;
                        for (int i=4; i>0; i--)
                        {
                            posx[(4*PieceCounter)-i]--;
                            temp.push_back(grid[(posx[(4*PieceCounter)-i])+1][((posy[(4*PieceCounter)-i]))]);
                            grid[(posx[(4*PieceCounter)-i]+1)][((posy[(4*PieceCounter)-i]))]=0;
                        }
                        for (int i=4; i>0; i--)
                        {
                            grid[(posx[(4*PieceCounter)-i])][(posy[(4*PieceCounter)-i])]=temp[4-i];
                        }
                    }
                    break;
                }
                }
            }
        }
        Uint32 ending_tick=SDL_GetTicks();
        if ((ending_tick-starting_tick)>delay) //for every delay
        {
            bool canMove;
            vector <int> temp;
            for (int i=4; i>0; i--)
            {
                //check if a block moving down would collide with anything
                canMove=xBoard.VcanMove(posx[(4*PieceCounter)-i],posy[(4*PieceCounter)-i]);
                if (canMove==false) //if it would collide, stop checking
                {
                    break;
                }
            }
            if (canMove==true)
            {
                for (int i=4; i>0; i--)
                {
                    posy[(4*PieceCounter)-i]++; //increase the vertical coordinate by 1
                    temp.push_back(grid[(posx[(4*PieceCounter)-i])][((posy[(4*PieceCounter)-i])-1)]);//new coordinate of each block
                    grid[(posx[(4*PieceCounter)-i])][((posy[(4*PieceCounter)-i])-1)]=0;//clear previous
                }
                for (int i=4; i>0; i--)
                {
                    grid[(posx[(4*PieceCounter)-i])][(posy[(4*PieceCounter)-i])]=temp[4-i];//assign new coordinate
                }
            }
            starting_tick=SDL_GetTicks();
            if (canMove==false) //if block cannot move down
            {
                xBoard.ClearLine();//check if line is full and needs to be cleared
                if (xBoard.Game_Over()==true) //check if game is over
                {
                    running=false;
                    break;
                }
                xGame.CreatePiece();//create new piece

            }
        }
        SDL_UpdateWindowSurface( window );
        SDL_BlitSurface(bckrnd, NULL, ScreenSurface, NULL );
        cap_framerate(starting_tick);
    }
    return 0;
}

