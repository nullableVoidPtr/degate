/*

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

#include <degate.h>
#include <RCVBlacklistExporter.h>
#include <FileSystem.h>
#include <ImageHelper.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <list>
#include <memory>


using namespace std;
using namespace degate;

void RCVBlacklistExporter::export_data(std::string const& filename,
				       RCBase::container_type const& violations) {

  std::string directory = get_basedir(filename);

  try {

    xmlpp::Document doc;

    xmlpp::Element * root_elem = doc.create_root_node("rc-blacklist");
    assert(root_elem != NULL);

    for(RCViolation_shptr rcv : violations) {
      add_rcv(root_elem, rcv);
    }

    doc.write_to_file_formatted(filename, "ISO-8859-1");

  }
  catch(const std::exception& ex) {
    std::cout << "Exception caught: " << ex.what() << std::endl;
    throw;
  }

}

void RCVBlacklistExporter::add_rcv(xmlpp::Element* root_elem,
				    RCViolation_shptr rcv) {

  xmlpp::Element* rcv_elem = root_elem->add_child("rc-violation");
  if(rcv_elem == NULL) throw(std::runtime_error("Failed to create node."));
  
  
  PlacedLogicModelObject_shptr o = rcv->get_object();    
  object_id_t new_oid = oid_rewriter->get_new_object_id(o->get_object_id());
  
  rcv_elem->set_attribute("object-id", number_to_string<object_id_t>(new_oid));
  rcv_elem->set_attribute("rc-violation-class", rcv->get_rc_violation_class());
  rcv_elem->set_attribute("severity", rcv->get_severity_as_string());
  rcv_elem->set_attribute("description", rcv->get_problem_description());

}

