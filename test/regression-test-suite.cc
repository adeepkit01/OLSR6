/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Ankit Deepak <adadeepak8@gmail.com>
 *          Shravya Ks <shravya.ks0@gmail.com>,
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

/*
 * PORT NOTE: This code was ported from ns-3 IPv4 implementation  (src/olsr).  Almost all
 * comments have also been ported from the same
 */


#include "hello-regression-test.h"
#include "tc-regression-test.h"

using namespace ns3;
using namespace olsr6;

class RegressionTestSuite : public TestSuite
{
public:
  RegressionTestSuite () : TestSuite ("routing-olsr6-regression", SYSTEM)
  {
    SetDataDir (NS_TEST_SOURCEDIR);
    AddTestCase (new HelloRegressionTest, TestCase::QUICK);
    AddTestCase (new TcRegressionTest, TestCase::QUICK);
  }
} g_olsr6RegressionTestSuite;
