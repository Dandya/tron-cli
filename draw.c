void draw_area(uint32_t *ptr, int xres, int yres, int scr_xres, int scr_yres)
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

int draw_car(uint32_t* ptr, char direction, uint32_t color, int src_xres)
{
    int was_overlap = 0;
    switch (direction)
    {
        case UP:
            for(int i = 0; i>-8; i--)
            {
                if( (ptr[-2+i*src_xres]+=color) != color ||
                    (ptr[-1+i*src_xres]+=color) != color ||
                    (ptr[i*src_xres]+=color) != color ||
                    (ptr[1+i*src_xres]+=color) != color ||
                    (ptr[2+i*src_xres]+=color) != color
                    )
                    was_overlap = 1;
            }
            break;
        case DOWN:
            for(int i = 0; i<8; i++)
            {
                if( (ptr[-2+i*src_xres]+=color) != color ||
                    (ptr[-1+i*src_xres]+=color) != color ||
                    (ptr[i*src_xres]+=color) != color ||
                    (ptr[1+i*src_xres]+=color) != color ||
                    (ptr[2+i*src_xres]+=color) != color
                    )
                    was_overlap = 1;
            }
            break;
        case LEFT:
            for(int i = 0; i>-8; i--)
            {
                if( (ptr[-2*src_xres+i]+=color) != color ||
                    (ptr[-1*src_xres+i]+=color) != color ||
                    (ptr[i]+=color) != color ||
                    (ptr[src_xres+i]+=color) != color ||
                    (ptr[2*src_xres+i]+=color) != color
                    )
                    was_overlap = 1;
            }
            break;
        case RIGHT:
            for(int i = 0; i<8; i++)
            {
                if( (ptr[-2*src_xres+i]+=color) != color ||
                    (ptr[-1*src_xres+i]+=color) != color ||
                    (ptr[i]+=color) != color ||
                    (ptr[src_xres+i]+=color) != color ||
                    (ptr[2*src_xres+i]+=color) != color
                    )
                    was_overlap = 1;
            }
            break;
    }
    return was_overlap;
}

int delete_car(uint32_t* ptr, char direction, uint32_t src_xres)
{
    switch (direction)
    {
        case UP:
            for(int i = 0; i>-8; i--)
            {
                ptr[-2+i*src_xres] = BLACK;
                ptr[-1+i*src_xres] = BLACK;
                ptr[i*src_xres] = BLACK;
                ptr[1+i*src_xres] = BLACK;
                ptr[2+i*src_xres] = BLACK;
            }
            break;
        case DOWN:
            for(int i = 0; i<8; i++)
            {
                ptr[-2+i*src_xres] = BLACK;
                ptr[-1+i*src_xres] = BLACK;
                ptr[i*src_xres] = BLACK;
                ptr[1+i*src_xres] = BLACK;
                ptr[2+i*src_xres] = BLACK;
            }
            break;
        case LEFT:
            for(int i = 0; i>-8; i--)
            {
                ptr[-2*src_xres+i] = BLACK;
                ptr[-1*src_xres+i] = BLACK;
                ptr[i] = BLACK;
                ptr[1*src_xres+i] = BLACK;
                ptr[2*src_xres+i] = BLACK
            }
            break;
        case RIGHT:
            for(int i = 0; i<8; i++)
            {
                ptr[-2*src_xres+i] = BLACK;
                ptr[-1*src_xres+i] = BLACK;
                ptr[i] = BLACK;
                ptr[1*src_xres+i] = BLACK;
                ptr[2*src_xres+i] = BLACK
            }
            break;
    }
}