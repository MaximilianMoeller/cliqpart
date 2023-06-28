//
// Created by max on 09.06.23.
//

#include "separator_factory.h"

#include <memory>
#include "separators/cubic_triangle_separator.h"
#include "separators/gw_separator.h"

unique_ptr<AbstractSeparator> SeparatorFactory::BuildSeparator(RunConfig &config, CompleteGraph &data) {
  //return  std::make_unique<CubicTriangleSeparator> (data);
  if (holds_alternative<CubicSeparatorConfig>(config.separator)) {
	PLOGD << "Creating new Cubic Triangle Separator!";
	return unique_ptr<AbstractSeparator>{new CubicTriangleSeparator(data)};
  } else if (holds_alternative<GWSeparatorConfig>(config.separator)) {
	int maxcut = get<GWSeparatorConfig>(config.separator).MAXCUT;
	PLOGD << "Creating new Grötschel-Wakabayashi Separator with MAXCUT parameter " << maxcut << " !";
	return unique_ptr<AbstractSeparator>{new GWSeparator(data, maxcut)};
  }

}
