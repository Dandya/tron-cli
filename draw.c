void draw_area(uint32_t *ptr, int xres, int yres, int scr_xres)
{
  for(int i = 0; i<xres+2; i++)
  {
      ptr[i] = WHITE;
      ptr[i+(yres+1)*scr_xres] = WHITE;
  }
  for(int i = 1; i<=yres; i++)
  {
      ptr[i*scr_xres] = WHITE;
      ptr[i*scr_xres+(xres+1)] = WHITE;
  }
}

int draw_car(uint32_t* ptr, char direction, uint32_t color, int scr_xres)
{
    int was_overlap = 0;
    switch (direction)
    {
        case UP:
            for(int i = 0; i>-8; i--)
            {
                /*
                if( (ptr[-2+i*src_xres]+=color) != color ||
                    (ptr[-1+i*src_xres]+=color) != color ||
                    (ptr[i*src_xres]+=color) != color ||
                    (ptr[1+i*src_xres]+=color) != color ||
                    (ptr[2+i*src_xres]+=color) != color
                    )
                    was_overlap = 1;*/
                for(int j = -2; j<=2; j++)
                {
                    if( ptr[j+i*scr_xres] == WHITE || 
                        ptr[j+i*scr_xres] == RED || 
                        ptr[j+i*scr_xres] == BLUE)
                        was_overlap = 1;
                    ptr[j+i*scr_xres] = color;
                }
            }
            break;
        case DOWN:
            for(int i = 0; i<8; i++)
            {
                /*
                if( (ptr[-2+i*src_xres]+=color) != color ||
                    (ptr[-1+i*src_xres]+=color) != color ||
                    (ptr[i*src_xres]+=color) != color ||
                    (ptr[1+i*src_xres]+=color) != color ||
                    (ptr[2+i*src_xres]+=color) != color
                    )
                    was_overlap = 1;*/
                for(int j = -2; j<=2; j++)
                {
                    if( ptr[j+i*scr_xres] == WHITE || 
                        ptr[j+i*scr_xres] == RED || 
                        ptr[j+i*scr_xres] == BLUE)
                        was_overlap = 1;
                    ptr[j+i*scr_xres] = color;
                }
            }
            break;
        case LEFT:
            for(int i = 0; i>-8; i--)
            {
                /*
                if( (ptr[-2*src_xres+i]+=color) != color ||
                    (ptr[-1*src_xres+i]+=color) != color ||
                    (ptr[i]+=color) != color ||
                    (ptr[src_xres+i]+=color) != color ||
                    (ptr[2*src_xres+i]+=color) != color
                    )
                    was_overlap = 1;*/
                for(int j = -2; j<=2; j++)
                {
                    if( ptr[i+j*scr_xres] == WHITE || 
                        ptr[i+j*scr_xres] == RED || 
                        ptr[i+j*scr_xres] == BLUE)
                        was_overlap = 1;
                    ptr[i+j*scr_xres] = color;
                }
            }
            break;
        case RIGHT:
            for(int i = 0; i<8; i++)
            {
                /*
                if( (ptr[-2*src_xres+i]+=color) != color ||
                    (ptr[-1*src_xres+i]+=color) != color ||
                    (ptr[i]+=color) != color ||
                    (ptr[src_xres+i]+=color) != color ||
                    (ptr[2*src_xres+i]+=color) != color
                    )
                    was_overlap = 1;*/
                for(int j = -2; j<=2; j++)
                {
                    if( ptr[i+j*scr_xres] == WHITE || 
                        ptr[i+j*scr_xres] == RED || 
                        ptr[i+j*scr_xres] == BLUE)
                        was_overlap = 1;
                    ptr[i+j*scr_xres] = color;
                }
            }
            break;
    }
    return was_overlap;
}

void delete_car(uint32_t* ptr, char direction, int scr_xres, uint32_t black)
{
    switch (direction)
    {
        case UP:
        {
            for(int i = 0; i>-8; i--)
            {
                ptr[-2+i*scr_xres] = black;
                ptr[-1+i*scr_xres] = black;
                ptr[i*scr_xres] = black;
                ptr[1+i*scr_xres] = black;
                ptr[2+i*scr_xres] = black;
            }
            break;
        }
        case DOWN:
        {
            for(int i = 0; i<8; i++)
            {
                ptr[-2+i*scr_xres] = black;
                ptr[-1+i*scr_xres] = black;
                ptr[i*scr_xres] = black;
                ptr[1+i*scr_xres] = black;
                ptr[2+i*scr_xres] = black;
            }
            break;
        }
        case LEFT:
        {
            for(int i = 0; i>-8; i--)
            {
                ptr[-2*scr_xres+i] = black;
                ptr[-1*scr_xres+i] = black;
                ptr[i] = black;
                ptr[scr_xres+i] = black;
                ptr[2*scr_xres+i] = black;
            }
            break;
        }
        case RIGHT:
        {
            for(int i = 0; i<8; i++)
            {
                ptr[-2*scr_xres+i] = black;
                ptr[-1*scr_xres+i] = black;
                ptr[i] = black;
                ptr[scr_xres+i] = black;
                ptr[2*scr_xres+i] = black;         
            }
            break;
        }
    }
}
