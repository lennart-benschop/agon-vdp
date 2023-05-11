//
// Title:	        Agon Video BIOS - Teletext Mode
// Author:        	Lennart Benschop
// Created:       	06/05/2023
// Last Updated:	07/05/2023
//
// Modinfo: 
// 07/05/2023:    Added support for windowing (cls and scroll methods). Implemented flash
// 11/06/2023:    Fixed hold graphics semantics

#pragma once

#include "ttxtfont.h"

#define COLOUR_BLACK   0x00
#define COLOUR_RED     0x30
#define COLOUR_GREEN   0x0C
#define COLOUR_YELLOW  0x3C
#define COLOUR_BLUE    0x03
#define COLOUR_MAGENTA 0x33
#define COLOUR_CYAN    0x0F
#define COLOUR_WHITE   0x3F

// State flags representing how a character must be displayed.
#define TTXT_STATE_FLAG_FLASH    0x01
#define TTXT_STATE_FLAG_CONCEAL  0x02
#define TTXT_STATE_FLAG_HEIGHT   0x04 // Double height active.
#define TTXT_STATE_FLAG_GRAPH    0x08
#define TTXT_STATE_FLAG_SEPARATE 0x10
#define TTXT_STATE_FLAG_HOLD     0x20
#define TTXT_STATE_FLAG_DHLOW    0x40 // We are on the lower row of a double-heigh.

#define IS_CONTROL(c)  ((c & 0x60)==0)

// The teletext video class
//
class agon_ttxt {	
	public:
    int init(fabgl::Canvas * Canvas);
    unsigned char get_screen_char(int x, int y);
    void draw_char(int x, int y, unsigned char c);
    void scroll();
    void cls();
    void flash(bool f);
    void set_window(int left, int bottom, int right, int top);
	private:
    fabgl::FontInfo font;
    fabgl::Canvas * Canvas;
    // Font bitmaps for normal height, top half of double height, bottom half of double height.
    unsigned char *font_data_norm, *font_data_top, *font_data_bottom; 
    unsigned char *screen_buf; // Buffer containing all bytes in teletext page.
    int lastRow, lastCol;
    unsigned char stateFlags;
    unsigned char heldGraph;
    RGB888 fg;
    RGB888 bg;
    int left, bottom, right, top;
    bool flashPhase;
    char dh_status[25]; // Double-height status per row: 0 none, 1 top half, 2 bottom half.
    void set_font_char(unsigned char dst, unsigned char src);
    void set_graph_char(unsigned char dst, unsigned char pat, bool contig);
    void setgrpbyte(int index, char dot, bool contig, bool inner);
    void display_char(int col, int row, unsigned char c);
    void process_line(int row, int col, bool redraw, bool do_flash);
};

void agon_ttxt::display_char(int col, int row, unsigned char c)
{
  switch (c)
  { // Translate some characters from ASCII to their code point in the Teletext font.
    case '#':
      c = '_';
      break;
    case '_':
      c = '\`';
      break;
    case '\`':
      c = '#';
      break;
  }  
  c = c & 0x7f;
  if ((this->stateFlags & TTXT_STATE_FLAG_GRAPH) && (c & 0x20)) // select graphics for chars 0x20--0x3f, 0x60--0x7f/
  { 
    if (this->stateFlags & TTXT_STATE_FLAG_SEPARATE)
      c = c + 128;
    else
      c = c + 96;
  }
  if ((this->stateFlags & TTXT_STATE_FLAG_CONCEAL) ||
      ((this->stateFlags & TTXT_STATE_FLAG_DHLOW) && !(this->stateFlags & TTXT_STATE_FLAG_HEIGHT)) ||
      ((this->stateFlags & TTXT_STATE_FLAG_FLASH) && !this->flashPhase))
    c = 32;
  this->Canvas->setPenColor(this->fg);
  this->Canvas->setBrushColor(this->bg);
  this->Canvas->drawChar(col*16, row*19, c);
}


// Process one line of text, parsing control codes.
// row -- rown number 0..24
// col -- process line up to column n.
// redraw, redraw the characters.
// do_flash. redraw only the characters that are flashing (redraw parameter must be false).
void agon_ttxt::process_line(int row, int col, bool redraw, bool do_flash)
{
  bool redrawNext = false;
  this->font.data = this->font_data_norm;
  if (this->dh_status[row] == 2) 
     this->stateFlags = TTXT_STATE_FLAG_DHLOW;
  else
     this->stateFlags = 0;
  if (redraw && this->dh_status[row] == 1)
  {
    // If we redraw the line, reconsider its double height status and that of the line below.
    this->dh_status[row] = 0;
    if (row < 24) this->dh_status[row+1] = 0;        
  }
  this->bg = colourLookup[COLOUR_BLACK];
  this->fg = colourLookup[COLOUR_WHITE];
  this->heldGraph = 0;

  for (int i=0; i<col; i++)
  {
    unsigned char c = this->screen_buf[row*40+i];
    if (IS_CONTROL(c))
    {
      // These control codes already take effect in the same cell (for held graphics or for background colour)
      switch(c & 0x7f)
      { 
      case 0x09:
        this->stateFlags &= ~TTXT_STATE_FLAG_FLASH;        
        if (do_flash) redraw = false;        
        break;
      case 0x0c:
        this->stateFlags &= ~TTXT_STATE_FLAG_HEIGHT;
        heldGraph=0;
        this->font.data = this->font_data_norm;
        break;                
      case 0x0d:
        this->stateFlags |= TTXT_STATE_FLAG_HEIGHT;
        heldGraph = 0;
        if (this->dh_status[row] == 0)
        {
          this->dh_status[row] = 1;
          if (row<24) {
            this->dh_status[row+1] = 2;
            redrawNext = true;
          }
          this->font.data = this->font_data_top;
        } else if (this->dh_status[row] == 1)
        {
            this->font.data = this->font_data_top;
        }
        else
        {
            this->font.data = this->font_data_bottom;
        }
        break;
      case 0x18:
        this->stateFlags |= TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x1C: 
        this->bg = colourLookup[COLOUR_BLACK];
        break;
      case 0x1D: 
        this->bg = this->fg;
        break;
      case 0x1E:
        this->stateFlags |= TTXT_STATE_FLAG_HOLD;
        break;
      }
      if (redraw)
      {
        this->Canvas->setPenColor(this->fg);
        this->Canvas->setBrushColor(this->bg);
        if (heldGraph == 0 || !(this->stateFlags & TTXT_STATE_FLAG_HOLD) ||
            (this->stateFlags & TTXT_STATE_FLAG_CONCEAL) ||
            ((this->stateFlags & TTXT_STATE_FLAG_DHLOW) && !(this->stateFlags & TTXT_STATE_FLAG_HEIGHT)) ||
            ((this->stateFlags & TTXT_STATE_FLAG_FLASH) && !this->flashPhase))
            this->Canvas->drawChar(i*16, row*19, 32);
         else   
            this->Canvas->drawChar(i*16, row*19, heldGraph);
      }
      // Thse control codes will take effect in the next cell.
      switch(c & 0x7f)
      { 
      case 0x01: 
        this->fg = colourLookup[COLOUR_RED];
        if (this->stateFlags & TTXT_STATE_FLAG_GRAPH) heldGraph=' ';
        this->stateFlags &= ~TTXT_STATE_FLAG_GRAPH;
        this->stateFlags &= ~TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x02: 
        this->fg = colourLookup[COLOUR_GREEN];
        if (this->stateFlags & TTXT_STATE_FLAG_GRAPH) heldGraph=' ';
        this->stateFlags &= ~TTXT_STATE_FLAG_GRAPH;
        this->stateFlags &= ~TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x03: 
        this->fg = colourLookup[COLOUR_YELLOW];
        if (this->stateFlags & TTXT_STATE_FLAG_GRAPH) heldGraph=' ';
        this->stateFlags &= ~TTXT_STATE_FLAG_GRAPH;
        this->stateFlags &= ~TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x04: 
        this->fg = colourLookup[COLOUR_BLUE];
        if (this->stateFlags & TTXT_STATE_FLAG_GRAPH) heldGraph=' ';
        this->stateFlags &= ~TTXT_STATE_FLAG_GRAPH;
        this->stateFlags &= ~TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x05: 
        this->fg = colourLookup[COLOUR_MAGENTA];
        if (this->stateFlags & TTXT_STATE_FLAG_GRAPH) heldGraph=' ';
        this->stateFlags &= ~TTXT_STATE_FLAG_GRAPH;
        this->stateFlags &= ~TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x06: 
        this->fg = colourLookup[COLOUR_CYAN];
        if (this->stateFlags & TTXT_STATE_FLAG_GRAPH) heldGraph=' ';
        this->stateFlags &= ~TTXT_STATE_FLAG_GRAPH;
        this->stateFlags &= ~TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x07: 
        this->fg = colourLookup[COLOUR_WHITE];
        if (this->stateFlags & TTXT_STATE_FLAG_GRAPH) heldGraph=' ';
        this->stateFlags &= ~TTXT_STATE_FLAG_GRAPH;
        this->stateFlags &= ~TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x08:
        this->stateFlags |= TTXT_STATE_FLAG_FLASH;
        if (do_flash) redraw = true;        
        break;
      case 0x11: 
        this->fg = colourLookup[COLOUR_RED];
        this->stateFlags |= TTXT_STATE_FLAG_GRAPH;
        this->stateFlags &= ~TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x12: 
        this->fg = colourLookup[COLOUR_GREEN];
        this->stateFlags |= TTXT_STATE_FLAG_GRAPH;
        this->stateFlags &= ~TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x13: 
        this->fg = colourLookup[COLOUR_YELLOW];
        this->stateFlags |= TTXT_STATE_FLAG_GRAPH;
        this->stateFlags &= ~TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x14: 
        this->fg = colourLookup[COLOUR_BLUE];
        this->stateFlags |= TTXT_STATE_FLAG_GRAPH;
        this->stateFlags &= ~TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x15: 
        this->fg = colourLookup[COLOUR_MAGENTA];
        this->stateFlags |= TTXT_STATE_FLAG_GRAPH;
        this->stateFlags &= ~TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x16: 
        this->fg = colourLookup[COLOUR_CYAN];
        this->stateFlags |= TTXT_STATE_FLAG_GRAPH;
        this->stateFlags &= ~TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x17: 
        this->fg = colourLookup[COLOUR_WHITE];
        this->stateFlags |= TTXT_STATE_FLAG_GRAPH;
        this->stateFlags &= ~TTXT_STATE_FLAG_CONCEAL;
        break;
      case 0x19:
        this->stateFlags &= ~TTXT_STATE_FLAG_SEPARATE;
        break;
      case 0x1A:
        this->stateFlags |= TTXT_STATE_FLAG_SEPARATE;
        break;
      case 0x1F:
        this->stateFlags &= ~TTXT_STATE_FLAG_HOLD;
        break;                   
      }
    }
    else 
    {
      heldGraph  = 0;
      if ((stateFlags & TTXT_STATE_FLAG_GRAPH) && (c & 0x20))
      {
         if (stateFlags & TTXT_STATE_FLAG_SEPARATE)
           heldGraph = (c & 0x7f) + 128;
         else
           heldGraph = (c & 0x7f) + 96;      
      } 
      if (redraw)
      {
        this->display_char(i, row, c);
      } 
    }
  }
  if (redraw && redrawNext && !do_flash)
     this->process_line(row+1, 40, true, false);
}

// Set single byte in a font character representing a graphic.
void agon_ttxt::setgrpbyte(int index, char dot, bool contig, bool inner)
{
  unsigned char b = 0x00;
  if (dot & 1)
  {
    if (contig)
      b = 0xff;
    else if (inner)
      b = 0x7e;
  }
  this->font_data_norm[index] = b;
}


// Store the required character from the ttxt_font into the font_data_norm member.
// dst - position in font_data_norm
// src - position in ttxtfont.
// Note: the ttxtfont array contains a 16x20 font stored as 16-bit values, 
// while the target is a 16x19 font stored as bytes. The bottom row (blank)  will not be copied.
void agon_ttxt::set_font_char(unsigned char dst, unsigned char src)
{
  for (int i = 0; i < 19; i++)
  {
    int w = ttxtfont[src*20 + i];
    this->font_data_norm[dst*38+2*i] = w & 0xff;
    this->font_data_norm[dst*38+2*i+1] = w >> 8;    
  }
}

// Store the required graphics character into the font_data_norm member/
// dst - position in font_data_norm
// pat - bit pattern of graphics character.
// contig - is character contiguoes.
void agon_ttxt::set_graph_char(unsigned char dst, unsigned char pat, bool contig)
{
  // Top row outer lines
  this->setgrpbyte(dst*38+0, pat>>0, contig, false);
  this->setgrpbyte(dst*38+1, pat>>1, contig, false);
  // Inner blocks of top row
  this->setgrpbyte(dst*38+2, pat>>0, contig, true);
  this->setgrpbyte(dst*38+3, pat>>1, contig, true);
  this->setgrpbyte(dst*38+4, pat>>0, contig, true);
  this->setgrpbyte(dst*38+5, pat>>1, contig, true);
  this->setgrpbyte(dst*38+6, pat>>0, contig, true);
  this->setgrpbyte(dst*38+7, pat>>1, contig, true);
  this->setgrpbyte(dst*38+8, pat>>0, contig, true);
  this->setgrpbyte(dst*38+9, pat>>1, contig, true);
  // Top row outer lines
  this->setgrpbyte(dst*38+10, pat>>0, contig, false);
  this->setgrpbyte(dst*38+11, pat>>1, contig, false);
  // Middle row outer lines
  this->setgrpbyte(dst*38+12, pat>>2, contig, false);
  this->setgrpbyte(dst*38+13, pat>>3, contig, false);
  // Middle row inner lines
  this->setgrpbyte(dst*38+14, pat>>2, contig, true);
  this->setgrpbyte(dst*38+15, pat>>3, contig, true);
  this->setgrpbyte(dst*38+16, pat>>2, contig, true);
  this->setgrpbyte(dst*38+17, pat>>3, contig, true);
  this->setgrpbyte(dst*38+18, pat>>2, contig, true);
  this->setgrpbyte(dst*38+19, pat>>3, contig, true);
  this->setgrpbyte(dst*38+20, pat>>2, contig, true);
  this->setgrpbyte(dst*38+21, pat>>3, contig, true);
  this->setgrpbyte(dst*38+22, pat>>2, contig, true);
  this->setgrpbyte(dst*38+23, pat>>3, contig, true);
  // Middle row outer lines
  this->setgrpbyte(dst*38+24, pat>>2, contig, false);
  this->setgrpbyte(dst*38+25, pat>>3, contig, false);
  // Bottom row, outer lines
  this->setgrpbyte(dst*38+26, pat>>4, contig, false);
  this->setgrpbyte(dst*38+27, pat>>5, contig, false);
  // Bottom row, inner lines
  this->setgrpbyte(dst*38+28, pat>>4, contig, true);
  this->setgrpbyte(dst*38+29, pat>>5, contig, true);
  this->setgrpbyte(dst*38+30, pat>>4, contig, true);
  this->setgrpbyte(dst*38+31, pat>>5, contig, true);
  this->setgrpbyte(dst*38+32, pat>>4, contig, true);
  this->setgrpbyte(dst*38+33, pat>>5, contig, true);
  this->setgrpbyte(dst*38+34, pat>>4, contig, true);
  this->setgrpbyte(dst*38+35, pat>>5, contig, true);
  // Bottom row, outer lines
  this->setgrpbyte(dst*38+36, pat>>4, contig, false);
  this->setgrpbyte(dst*38+37, pat>>5, contig, false);
}


int agon_ttxt::init(fabgl::Canvas * Canvas)
{
  if (this->font_data_norm == NULL)
  {
    this->font_data_norm=(unsigned char*)heap_caps_malloc(40*256, MALLOC_CAP_8BIT|MALLOC_CAP_SPIRAM);
    if (this->font_data_norm == NULL)
      return -1;
    for (int i=32; i<127; i++)
      this->set_font_char(i, i);
    // Set the characters that differ from standard ASCII (in UK teletext char set).
    this->set_font_char('#', 163); // Pounds sign.
    this->set_font_char('[', 143); // Left arrow.
    this->set_font_char('\\', 189); // 1/2 fraction
    this->set_font_char(']', 144); // Right arrow.
    this->set_font_char('^', 141); // Up arrow.
    this->set_font_char('_', '#'); // Hash mark.
    this->set_font_char('`', 151); // Em-dash.
    this->set_font_char('{', 188); // 1/4 fraction.
    this->set_font_char('|', 157); // Dpuble vertical bar
    this->set_font_char('}', 190); // 3/4 fraction.
    this->set_font_char('~', 247); // Division.
    this->set_font_char(127, 129); // Block.
    // Positions 128..255 are for graphics characters, both contiguous and separated.
    for (int i=0; i<32; i++)
    {
      this->set_graph_char(128+i, i, true); // Contiguous graphics characters
      this->set_graph_char(160+i, i, false); // Separate graphics characters.
      this->set_graph_char(192+i, 32+i, true);
      this->set_graph_char(224+i, 32+i, false);
    }
  }
  if (this->font_data_top == NULL)
  {
    this->font_data_top=(unsigned char*)heap_caps_malloc(40*256, MALLOC_CAP_8BIT|MALLOC_CAP_SPIRAM);
    if (this->font_data_top == NULL)
      return -1;
    for (int i=32; i<256; i++)
    {
      unsigned char *p = this->font_data_norm+38*i;
      for (int j=0; j<19; j++)
      {
        this->font_data_top[i*38+j*2] = *p;
        this->font_data_top[i*38+j*2+1] = *(p+1);
        if (j%2 == 1) p+=2;
      }
    }
  }
  if (this->font_data_bottom == NULL)
  {
    this->font_data_bottom=(unsigned char*)heap_caps_malloc(40*256, MALLOC_CAP_8BIT|MALLOC_CAP_SPIRAM);
    if (this->font_data_bottom == NULL)
      return -1;
    for (int i=32; i<256; i++)
    {
      unsigned char *p = this->font_data_norm+38*i+18;
      for (int j=0; j<19; j++)
      {
        this->font_data_bottom[i*38+j*2] = *p;
        this->font_data_bottom[i*38+j*2+1] = *(p+1);
        if (j%2 == 0) p+=2;
      }
    }
  }
  if (this->screen_buf == NULL)
  {
    this->screen_buf=(unsigned char *)heap_caps_malloc(1000, MALLOC_CAP_8BIT|MALLOC_CAP_SPIRAM);
    if (this->screen_buf == NULL)
      return -1;      
  }
  this->font.pointSize = 12;
  this->font.width = 16;
  this->font.height = 19;
  this->font.ascent = 12;
  this->font.weight = 400;
  this->font.charset = 255;
  this->font.codepage = 1252;
  this->Canvas = Canvas;
  Canvas->selectFont(&this->font);
  Canvas->setGlyphOptions(GlyphOptions().FillBackground(true));
  this->set_window(0, 24, 39, 0);
  this->cls();
  return 0;
}

unsigned char agon_ttxt::get_screen_char(int x, int y)
{
  x=x/this->Canvas->getFontInfo()->width;
  y=y/this->Canvas->getFontInfo()->height;
  if (x<0 || x>39 || y<0 || y>24)
    return 0;
  else
    return this->screen_buf[x+y*40];
}

void agon_ttxt::draw_char(int x, int y, unsigned char c)
{
  int cx=x/this->Canvas->getFontInfo()->width;
  int cy=y/this->Canvas->getFontInfo()->height;
  if (cx<0 || cx>39 || cy<0 || cy>24)
    return;
  unsigned char oldb = this->screen_buf[cx+cy*40];
  this->screen_buf[cx+cy*40] = c;
  // Determine how much to render.
  if (IS_CONTROL(c) || IS_CONTROL(oldb))
  {
     // If removing or adding control character, redraw entire line.
     this->process_line(cy, 40, true, false); 
     this->lastRow = -1;
     return;
  }
  else if (cx != this->lastCol+1 || cy != this->lastRow)
  {
      // If not continuing on same line after last drawn character, parse all control chars.
      this->process_line(cy, cx, false, false); 
      this->lastCol = cx;
      this->lastRow = cy;
      
  } 
  this->display_char(cx, cy, c);
}

void agon_ttxt::scroll()
{
  if (this->left==0 && this->right==39 && this->top==0 && this->bottom==24)
  {
    /* Do the full screen */
    memcpy(this->screen_buf, this->screen_buf+40, 960);
    memset(this->screen_buf+960, ' ', 40);
    this->lastRow--;
    memcpy(this->dh_status, this->dh_status+1, 24);
    if (this->dh_status[23] == 1) 
      this->dh_status[24] = 2;
    else
      this->dh_status[24] = 0;
     this->Canvas->scroll(0, -19);
     if (this->dh_status[0] == 2)
     {
        this->dh_status[0] = 1;
        process_line(0, 40, true, false);
        lastRow = -1;
     }     
  }
  else
  {
    for (int row = this->top; row < this->bottom; row++)
    {
      memcpy(this->screen_buf+40*row+this->left,this->screen_buf+40*(row+1)+this->left , this->right+1-this->left);
      this->process_line(row, 40, true, false);      
    }
    memset(this->screen_buf+40*this->bottom+this->left, ' ', this->right+1-this->left);
    this->process_line(this->bottom, 40, true, false);
  }
}

void agon_ttxt::cls()
{
  this->lastRow = -1;
  this->lastCol = -1;
  if (this->left==0 && this->right==39 && this->top==0 && this->bottom==24)
  {
      /* Do the full screen */
      memset(this->screen_buf, ' ', 1000);
      memset(this->dh_status, 0, 25);
      this->Canvas->clear();
  }
  else
  {
    for (int row=this->top; row <= this->bottom; row++)
    {
      memset(this->screen_buf+40*row+this->left, ' ', this->right+1-this->left);
      this->process_line(row, 40, true, false);
    }
  }
}


void agon_ttxt::flash(bool f)
{
  flashPhase = f;
  bool fUpdated = false;
  RGB888 oldbg = this->bg;
  for (int i = 0; i < 24; i++)
  {
    for (int j = 0; j < 40; j++)
    {
      if ((screen_buf[i*40+j] & 0x7f) == 0x08)
      {
        // Scan/redraw the line if there is any flash control code.
        process_line(i, 40, false, true); 
        fUpdated = true;
        break; 
      }
    }
  }
  if (fUpdated)
  {
    if (lastRow >= 0) 
      process_line(lastRow, lastCol, false, false);
    this->Canvas->setBrushColor(oldbg);
  }
}

// Set the window parametes of the Teletext mode.
// This only rules the cls and scroll methods.
void agon_ttxt::set_window(int left, int bottom, int right, int top)
{
  this->left = left;
  this->bottom = bottom;
  this->right = right;
  this->top = top;  
}
