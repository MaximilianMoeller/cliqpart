//
// Created by max on 09.06.23.
//

#include "separator_factory.h"

#include <memory>
#include <variant>
#include "run_config.h"
#include "separators/triangle_separator.h"
#include "separators/st_separator.h"

vector<unique_ptr<IAbstractSeparator>> SeparatorFactory::BuildSeparator(int degree, const RunConfig &config) {

  vector<unique_ptr<IAbstractSeparator>> res;

  for (ConfigVariant sep_config : config.separator_configs) {
	if (holds_alternative<TriangleSeparatorConfig>(sep_config)) {
	  PLOGD << "Creating new Δ separator.";
	  res.emplace_back(make_unique<TriangleSeparator>(degree, get<TriangleSeparatorConfig>(sep_config)));
	} else if (holds_alternative<StSeparatorConfig>(sep_config)) {
	  PLOGD << "Creating new [S:T] separator.";
	  res.emplace_back(make_unique<StSeparator>(degree, get<StSeparatorConfig>(sep_config)));
	}

  }

  return res;
}
