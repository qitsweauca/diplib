/*
 * DIPlib 3.0
 * This file contains common functions for the Gaussian filters.
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace dip {

namespace {

inline dip::uint HalfGaussianSize(
      dfloat sigma,
      dip::uint order,
      dfloat truncation
) {
   return static_cast< dip::uint >( std::ceil( ( truncation + 0.5 * order ) * sigma ));
}

} // namespace

} // namespace dip
