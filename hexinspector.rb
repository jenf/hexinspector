require "hexfile.rb"
require 'rubygems'
require 'ncurses'

h=open(ARGV[0])
file=HexInspectorFile.new(h.read)
class HexInspector
  def initialize(window,file)
    @window = window
    @window_size_changed = true
    @width = 16
    @file = file
    @offset=0
  end

  def dump_hex(file,lines,offset=0,width=16)
  i=0
  (offset..offset+(lines*width)).step(width) do |x|
    str=""
    str << "%08x " % x
    (0..width-1).each do |y|
      str << " " if (y % 8)==0
      str << "%02x "% file[x+y] if file[x+y]!=nil
      str << "  " if file[x+y]==nil
    end
    Ncurses.mvaddstr(i,0,str)
    i+=1
  end
  end


  def show_ruler()
    bit8_val=@file[@offset]
    Ncurses.mvaddstr(@height-1,0,"Byte %i/0x%08x Val : %03i/0x%02x/0%03o" % [@offset,@offset,bit8_val,bit8_val,bit8_val])
  end

  def show_substrs()
    (matches,len) = @file.find_substr(@file,@offset)
    Ncurses.mvaddstr(@height-2,0,"Len %i" % len)
  end
  def main_loop
   while true
    if @window_size_changed
      @height=@window.getmaxy
      @pager_lines=@height-5
      @window_size_changed=false
    end
    Ncurses.clear
    dump_hex(@file, @pager_lines, @offset) 
    show_substrs()
    show_ruler()
    Ncurses.refresh

    case @window.getch
     when Ncurses::KEY_LEFT
       @offset-=1
       @offset=0 if @offset<0
     when Ncurses::KEY_RIGHT
       @offset+=1
     when Ncurses::KEY_UP
      @offset-=@width
      @offset=0 if @offset<0
     when Ncurses::KEY_DOWN
      @offset+=@width
     when Ncurses::KEY_RESIZE
       @window_size_changed=true
    end
   end
 end
end

line_offset=0
Ncurses.initscr
Ncurses.cbreak
Ncurses.noecho
Ncurses.keypad(Ncurses.stdscr, true)

inspector = HexInspector.new(Ncurses.stdscr, file)
begin
  inspector.main_loop
ensure
  Ncurses.curs_set(1)
  Ncurses.endwin()
end
