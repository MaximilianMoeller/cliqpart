//
// Created by max on 09.06.23.
//

#include "separator_factory.h"

#include <memory>
#include "separators/triangle_separator.h"
#include "separators/st_separator.h"

vector<unique_ptr<AbstractSeparator>> SeparatorFactory::BuildSeparator(RunConfig &config, ModelWrapper &data) {

  vector<unique_ptr<AbstractSeparator>> res;

  for (auto SepConfig: config.separators){
	if (holds_alternative<TriangleSeparatorConfig>(SepConfig)) {
	  int maxcut = get<TriangleSeparatorConfig>(SepConfig).MAXCUT;
	  PLOGD << "Creating new Δ separator with MAXCUT parameter " << maxcut << " !" ;
	  res.emplace_back(make_unique<TriangleSeparator>(data, config.tolerance, maxcut));
	} else if (holds_alternative<ST_SeparatorConfig>(SepConfig)) {
	  int maxcut = get<ST_SeparatorConfig>(SepConfig).MAXCUT;
	  PLOGD << "Creating new [S:T] separator with MAXCUT parameter " << maxcut << " !" ;
	  res.emplace_back(make_unique<ST_Separator>(data, config.tolerance, maxcut, ST_Separator_Variant::GW2));
	}

  }

  return res;
}
