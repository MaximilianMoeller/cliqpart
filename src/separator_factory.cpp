//
// Created by max on 09.06.23.
//

#include "separator_factory.h"

#include <memory>
#include "separators/triangle_separator.h"
#include "separators/st_separator.h"

vector<unique_ptr<AbstractSeparator>> SeparatorFactory::BuildSeparator(RunConfig &config, CompleteGraph &data) {
  //return  std::make_unique<TriangleSeparator> (data);
  vector<unique_ptr<AbstractSeparator>> res;
  for (auto SepConfig: config.separators){
	if (holds_alternative<TriangleSeparatorConfig>(SepConfig)) {
	  int maxcut = get<TriangleSeparatorConfig>(SepConfig).MAXCUT;
	  PLOGD << "Creating new Δ separator with MAXCUT parameter " << maxcut << " !" ;
	  res.emplace_back(unique_ptr<AbstractSeparator>{new TriangleSeparator(data)});
	} else if (holds_alternative<ST_SeparatorConfig>(SepConfig)) {
	  int maxcut = get<TriangleSeparatorConfig>(SepConfig).MAXCUT;
	  PLOGD << "Creating new [S:T] separator with MAXCUT parameter " << maxcut << " !" ;
	  res.emplace_back(unique_ptr<AbstractSeparator>{new ST_Separator(data, maxcut)});
	}

  }

}
