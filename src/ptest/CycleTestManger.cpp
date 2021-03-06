/*
 * CycleTestManger.cpp
 *
 *  Created on: 2014��12��30��
 *      Author: ranger.shi
 */


#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "miner.h"
#include "uint256.h"
#include "util.h"
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "json/json_spirit_writer_template.h"
#include "CDarkAndAnony.h"
#include "rpcclient.h"

#include "CycleP2PBet_test.h"

using namespace std;
using namespace boost;
using namespace json_spirit;
#include "CycleTestBase.h"
#include "CycleTestManger.h"
#include "CycleSesureTrade_tests.h"
class CycleTestManger {

	vector<std::shared_ptr<CycleTestBase> > vTest;

public:
	CycleTestManger(){
//		vTest.push_back(std::make_shared<CTestSesureTrade>()) ;
//		vTest.push_back(std::make_shared<CTestSesureTrade>()) ;
//		vTest.push_back(std::make_shared<CTestSesureTrade>()) ;
//		vTest.push_back(std::make_shared<CDarkAndAnony>()) ;
//		vTest.push_back(std::make_shared<CDarkAndAnony>()) ;
		vTest.push_back(std::make_shared<CTestBetTx>()) ;
//		string dir = SysCfg().GetArg("rsetdir", "d:\\bitcoin");
//		if (dir != "d:\\bitcoin") {
//			char *argv[] = { "progname", (char*)dir.c_str() };
//			int argc = sizeof(argv) / sizeof(char*);
//			CBaseParams::IntialParams(argc, argv);
//		}

	};
	void run() {

		while (vTest.size() > 0) {
			for (auto it = vTest.begin(); it != vTest.end();) {
				bool flag = false;
				try {
					if (it->get()->run() == end_state) {
						flag = true;
					};
				} catch (...) {
					flag = true;
				}

				if (flag)
					it = vTest.erase(it);
				else
					++it;

			}
			MilliSleep(1000);

		}
	}
	virtual ~CycleTestManger(){};

};


BOOST_FIXTURE_TEST_SUITE(CycleTest,CycleTestManger)

BOOST_FIXTURE_TEST_CASE(Cycle,CycleTestManger)
{
	run();
}

BOOST_AUTO_TEST_SUITE_END()
