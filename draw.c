int draw_car(uint32_t* ptr, int x, int y, char direction, uint32_t color);
int delete_car(uint32_t* ptr, int x, int y, char direction);

void draw_area(uint32_t *ptr, int xres, int yres, int scr_xres, int scr_yres)
{
  for(int i = 0; i<xres+2; i++)
  {
      ptr[i] = 0x00FFFFFF;
      ptr[i+(yres+1)*scr_yres] = 0x00FFFFFF;
  }
  for(int i = 0; i<yres; i++)
  {
      ptr[i*scr_yres] = 0x00FFFFFF;
      ptr[i*scr_yres+(xres+1)] = 0x00FFFFFF;
  }
}

