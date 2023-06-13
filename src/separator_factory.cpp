//
// Created by max on 09.06.23.
//

#include "separator_factory.h"
#include "separators/cubic_triangle_separator.h"
unique_ptr<AbstractSeparator> SeparatorFactory::BuildSeparator(RunConfig &config, CompleteGraph &data) {
  // TODO add configuration support (more separators, which heuristics with which values, …)
  return unique_ptr<CubicTriangleSeparator> {new CubicTriangleSeparator(data)};
}
