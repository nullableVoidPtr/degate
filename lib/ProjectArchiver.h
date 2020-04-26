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

#ifndef __PROJECTARCHIVER_H__
#define __PROJECTARCHIVER_H__

#include <degate.h>
#include <Project.h>

#include <stdexcept>

namespace degate {

  /**
   * Export a project directory as a ZIP archive.
   *
   */

  class ProjectArchiver {

  private:

    void add_single_file(struct zip * zip_archive,
			 std::filesystem::path const& archive_file,
			 std::filesystem::path const& base_dir_path,
			 std::filesystem::path const& file,
			 std::filesystem::path const& prepend_dir) const;


      void add_directory(struct zip * zip_archive,
			 std::filesystem::path const& archive_file,
			 std::filesystem::path const& base_dir_path,
			 std::filesystem::path const& dir,
			 std::filesystem::path const& prepend_dir) const;

  public:
    ProjectArchiver() {}
    ~ProjectArchiver() {}

    void export_data(std::filesystem::path const& project_dir,
		     std::filesystem::path const& archive_file,
		     std::filesystem::path const& prepend_dir) const;
  };

}

#endif
