int delete_car(uint32_t* ptr, int x, int y, char direction);

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
    switch (direction)
    {
        case UP:
            for(int i = 0; i>-8; i--)
            {
                for(int j)
                ptr[-2+i*src_xres]=ptr[]
            }
            break;
        case DOWN:
            
            break;
        case LEFT:
            
            break;
        case RIGHT:
            
            break;
    }
    return 0;
}

