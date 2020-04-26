/* -*-c++-*-

  This file is part of the IC reverse engineering tool degate.

  Copyright 2008, 2009, 2010 by Martin Schobert

  Degate is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  Degate is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with degate. If not, see <http://www.gnu.org/licenses/>.

*/

#include "TerminalWin.h"


#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <boost/format.hpp>
#include <boost/algorithm/string/join.hpp>


using namespace degate;

TerminalWin::TerminalWin(Gtk::Window * parent) :
  GladeFileLoader("terminal.glade", "terminal_win"), cmd_state(DONE) {

  if(get_dialog()) {
    
    // connect signals
    get_widget("close_button", pCloseButton);
    assert(pCloseButton != NULL);
    if(pCloseButton != NULL)
      pCloseButton->signal_clicked().connect(sigc::mem_fun(*this, 
							   &TerminalWin::on_close_button_clicked));
    

    get_widget("textview", textview);
    assert(textview != NULL);
    if(textview != NULL) {
      textview->modify_font(Pango::FontDescription("courier"));
    }

  }
}



TerminalWin::~TerminalWin() {
  Glib::spawn_close_pid(pid);
}




void TerminalWin::run(cmd_type const & cmd, std::string const & working_dir) {
  this->working_dir = working_dir;
  exec_program(cmd);
  get_dialog()->show();
}

void TerminalWin::run(std::list<cmd_type > const& cmds, std::string const & working_dir) {
  this->cmds = cmds;
  this->working_dir = working_dir;
  run_next_command();
  get_dialog()->show();
}

void TerminalWin::run_next_command() {
  assert(cmd_state == DONE);
  if(cmds.size() > 0) {
    cmd_type cmd = cmds.front();
    cmds.pop_front();
    exec_program(cmd);
  }
}


void TerminalWin::on_close_button_clicked() {
  get_dialog()->hide();
}


void TerminalWin::exec_program(std::list<std::string> cmd) {

  cmd_state = RUNNING;

  for(std::string const& s : cmd) {
    std::cout << "[" << s << "]" << std::endl;
  }

  append_text("Run command: " + boost::algorithm::join(cmd, " ") + "\n");

  try {

    Glib::spawn_async_with_pipes(working_dir.empty() ? Glib::get_current_dir() : working_dir, 
				 cmd,
				 Glib::SPAWN_SEARCH_PATH, 
				 sigc::slot< void >(), // const sigc::slot< void > &  child_setup = sigc::slot< void >(),
				 &pid, 
				 NULL, &fd_stdout, &fd_stderr); 
       
    Glib::signal_io().connect(sigc::mem_fun(*this, &TerminalWin::on_read_stdout), fd_stdout, 
			      Glib::IO_IN | Glib::IO_HUP | Glib::IO_ERR | Glib::IO_NVAL);
    Glib::signal_io().connect(sigc::mem_fun(*this, &TerminalWin::on_read_stderr), fd_stderr, 
			      Glib::IO_IN | Glib::IO_HUP | Glib::IO_ERR | Glib::IO_NVAL);

  }
  catch(Glib::SpawnError const& ex) {
    cmd_state = FAILED;

    boost::format f("Failed to launch command \"%1%\". Error was: \"%2%\"");
    f % boost::algorithm::join(cmd, " ") % ex.what().c_str();

    Gtk::MessageDialog dialog(f.str(), true, Gtk::MESSAGE_ERROR);
    dialog.set_title("Execution Error");
    dialog.run();
  }
}


void TerminalWin::append_text(std::string const& s) {
  std::cout << "append text: " << s << std::endl;
  textview->get_buffer()->insert(textview->get_buffer()->end(), 
				 Glib::convert_with_fallback(s, "UTF-8", "ISO-8859-1"));
  
}


bool TerminalWin::on_read_stdout(Glib::IOCondition condition) {
  std::cout << "on_read_stdout()\n";
  if(condition & Glib::IO_IN)
    return read_and_append(fd_stdout, buf_stdout);
  else return handle_io(condition, buf_stdout);
}

bool TerminalWin::on_read_stderr(Glib::IOCondition condition) {
  std::cout << "on_read_stderr()\n";
  if(condition & Glib::IO_IN)
    return read_and_append(fd_stderr, buf_stderr);
  else return handle_io(condition, buf_stderr);
}

bool TerminalWin::read_and_append(int fd, std::string & strbuf) {
  char buf[1024];
  memset(buf, 0, sizeof(buf));
  int n = read(fd, buf, sizeof(buf));
  
  strbuf.append(buf, n);

  size_t pos;
  while((pos = strbuf.find_first_of("\n")) != std::string::npos) {

    std::string line = strbuf.substr(0, pos+1);
      
    std::cout << "get line: [" <<  line << "]" << std::endl;
    append_text(line);
    
    strbuf.erase(0, pos+1); // erase line from buffer
  }

  return (n > 0);
}

bool TerminalWin::handle_io(Glib::IOCondition condition, std::string & strbuf) {

  if(condition & Glib::IO_NVAL) {
    append_text("\n--- IO_NVAL\n");

    cmd_state = FAILED;
    return false;
  }
  else if(condition & Glib::IO_ERR) {
    append_text(strbuf);
    append_text("\n--- IO_ERR\n");
    cmd_state = FAILED;
    return false;
  }
  else if(condition & Glib::IO_HUP) {
    append_text(strbuf);
    append_text("\n--- IO_HUP\n");
    cmd_state = DONE;
    run_next_command();
    return false;
  }

  return false;
}

