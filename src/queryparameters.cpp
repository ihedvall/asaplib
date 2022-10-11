/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "queryparameters.h"

namespace asap3 {

QueryParameters::QueryParameters() {}

QueryParameters::~QueryParameters() { QueryParameters::Stop(); }

}