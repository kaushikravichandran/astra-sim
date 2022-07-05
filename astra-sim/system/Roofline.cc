/******************************************************************************
This source code is licensed under the MIT license found in the
LICENSE file in the root directory of this source tree.
*******************************************************************************/

#include "astra-sim/system/Roofline.hh"

#include <algorithm>

using namespace std;
using namespace AstraSim;

Roofline::Roofline() {
  bandwidth = 0;
  peak_perf = 0;
}

void Roofline::set_bandwidth(double bandwidth) {
  this->bandwidth = bandwidth;
}

void Roofline::set_peak_perf(double peak_perf) {
  this->peak_perf = peak_perf;
}

uint64_t Roofline::get_perf(double operational_intensity) {
  return min(static_cast<uint64_t>(bandwidth * operational_intensity),
             peak_perf);
}
