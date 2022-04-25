int delete_car(uint32_t* ptr, int x, int y, char direction);

void draw_area(uint32_t *ptr, int xres, int yres, int scr_xres, int scr_yres)
{
  for(int i = 0; i<xres+2; i++)
  {
      ptr[i] = 0x00FFFFFF;
      ptr[i+(yres+1)*scr_xres] = 0x00FFFFFF;
  }
  for(int i = 1; i<=yres; i++)
  {
      ptr[i*scr_xres] = 0x00FFFFFF;
      ptr[i*scr_xres+(xres+1)] = 0x00FFFFFF;
  }
}

int draw_car(uint32_t* ptr, int x, int y, char direction, uint32_t color)
{
    return 0;
}

