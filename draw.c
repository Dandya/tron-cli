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

void delete_car(uint32_t* ptr, char direction, int scr_xres, uint32_t background_color)
{
    switch (direction)
    {
        case UP:
        {
            for(int i = 0; i>-8; i--)
            {
                ptr[-2+i*scr_xres] = background_color;
                ptr[-1+i*scr_xres] = background_color;
                ptr[i*scr_xres] = background_color;
                ptr[1+i*scr_xres] = background_color;
                ptr[2+i*scr_xres] = background_color;
            }
            break;
        }
        case DOWN:
        {
            for(int i = 0; i<8; i++)
            {
                ptr[-2+i*scr_xres] = background_color;
                ptr[-1+i*scr_xres] = background_color;
                ptr[i*scr_xres] = background_color;
                ptr[1+i*scr_xres] = background_color;
                ptr[2+i*scr_xres] = background_color;
            }
            break;
        }
        case LEFT:
        {
            for(int i = 0; i>-8; i--)
            {
                ptr[-2*scr_xres+i] = background_color;
                ptr[-1*scr_xres+i] = background_color;
                ptr[i] = background_color;
                ptr[scr_xres+i] = background_color;
                ptr[2*scr_xres+i] = background_color;
            }
            break;
        }
        case RIGHT:
        {
            for(int i = 0; i<8; i++)
            {
                ptr[-2*scr_xres+i] = background_color;
                ptr[-1*scr_xres+i] = background_color;
                ptr[i] = background_color;
                ptr[scr_xres+i] = background_color;
                ptr[2*scr_xres+i] = background_color;         
            }
            break;
        }
    }
}
