//
// Created by max on 09.06.23.
//

#include "run_config.h"
RunConfig RunConfig::FromFile(const string &path) {
  toml::table tbl;

  try{
	tbl = toml::parse_file(path);

	if (!tbl.contains("degree") || !tbl["degree"].is_integer()){
	  PLOGF << path << " must contain a key 'degree' with an integer value!";
	  exit(-1);
	}
	if (!tbl.contains("offset")){
	  PLOGI << path << ": no offset was given, assuming 0.";
	}

	return RunConfig {.degree = tbl["degree"].value_or(1), .obj_offset = tbl["offset"].value_or(0.0)};
  }
  catch (const toml::parse_error& err){
	PLOGF << "Could not parse run_config.toml. Error was: " << endl << err << endl;
  }
}
