// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"

#include "assert.h"
#include "core.h"
#include "protocol.h"
#include "util.h"
#include "key.h"
#include "tx.h"

#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/case_conv.hpp> // for to_lower()
#include <boost/algorithm/string/predicate.hpp> // for startswith() and endswith()
#include <boost/filesystem.hpp>
using namespace boost::assign;
using namespace std;

map<string, string> CBaseParams::m_mapArgs;
map<string, vector<string> > CBaseParams::m_mapMultiArgs;



//	{
//        "addr" : "mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA",
//        "RegID" : "0-9",
//        "RegID2" : "000000000900"
//    },
//    {
//        "addr" : "mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5",
//        "RegID" : "0-5",
//        "RegID2" : "000000000500"
//    },
//    {
//        "addr" : "mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y",
//        "RegID" : "0-3",
//        "RegID2" : "000000000300"
//    },
//    {
//        "addr" : "n4muwAThwzWvuLUh74nL3KYwujhihke1Kb",
//        "RegID" : "0-8",
//        "RegID2" : "000000000800"
//    },
//    {
//        "addr" : "mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7",
//        "RegID" : "0-7",
//        "RegID2" : "000000000700"
//    },
//    {
//        "addr" : "moZJZgsGFC4qvwRdjzS7Bj3fHrtpUfEVEE",
//        "RegID" : "0-4",
//        "RegID2" : "000000000400"
//    },
//    {
//        "addr" : "mfzdtseoKfMpTd8V9N2xETEqUSWRujndgZ",
//        "RegID" : " ",
//        "RegID2" : "000000000000"
//    },
//    {
//        "addr" : "mjSwCwMsvtKczMfta1tvr78z2FTsZA1JKw",
//        "RegID" : "0-1",
//        "RegID2" : "000000000100"
//    },
//    {
//        "addr" : "msdDQ1SXNmknrLuTDivmJiavu5J9VyX9fV",
//        "RegID" : "0-10",
//        "RegID2" : "000000000a00"
//    },
//    {
//        "addr" : "mo51PMpnadiFx5JcZaeUdWBa4ngLBVgoGz",
//        "RegID" : " ",
//        "RegID2" : "000000000000"
//    },
//    {
//        "addr" : "mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v",
//        "RegID" : "0-6",
//        "RegID2" : "000000000600"
//    },
//    {
//        "addr" : "mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc",
//        "RegID" : "0-2",
//        "RegID2" : "000000000200"
//    }
//
// Main network
//
string initPubKey[] = { //

		"03d308757fc1f8efd69f2da329db560cd7d3cba951eb09786c375cf1709f9165ba",
		"024126ccf4b5f6463a3f874f234b77d02e9f5c2057c6c382160dc17c7f9ba2b333",
		"0221b571330617821e8c508416b90988e81e8dc8623576b8f6e942797e9f381111",
		"02e5c2fbea1055139e3d46621ef49aede5eb3ca1629fc3520986c5aba203706e74",
		"0295c3d0f4a913fd8dccf7a0782ae59fae282057169214a136b3722d5299248683",
		"03a616686dd872e301eb317a1ff4b53530d69492e8a39f6468afbd018043623166",
		"02ac35dabc82e297697e9774a2a3f9b1ddd5dc536da0c9bba7f5e95eb525aa4706",
		"0324da799ffc36177c9d401a81b6a1b4d90e7553ac4165fb263bbb8b60940a2b1d",
		"0311ca277a0f3880eefed349727cc954354c4e539b8128c79f18a87c6f2b979186",
		"02be4a840cf29bcbf84e3b7a0243adff77a32b9d76083ff254c1110734ea6b5792",
		"029bbfd75711ac071b361e133679d21c1cc6314a6c3fd71e91880956b91e640948",

		"02a3e7998d3f8dd6dedfc6bf06e46adf98041bf4ca8e9af103d62f85c6f0a0a9bf",
		"024ee551da4a0ca765f21a2c9d33ad61826f36eba8912ad80e8d5bf75c397f3ee2",
		"03e6cda0f68b8028a74bcbbde1a164a022dad8956eabaa61bd40935d06c6fb55a4",
		"03ec7e8f2521cc5d88be7e603c35ba9d4ecda09abe8f96a28514956c16ac7d9019",
		"03e7f41495038767d7eb522938de5bc4556ef2e1075432245b9fc0e348c0516baf",
		"03a61b40def5330abbb9f7491b11d586283b5fd2fb2e5cd546b0265f21e9cf9356",
		"025a1c65b72c72569559edf54491dcf45e7ed0299886ee3eaa3cf5d5765b5b606b",
		"035a998c0adb99003c0552ed44ac3f46e80f542b2d26cb94dd3356027b1c844b99",
		"03f61c32ccc409ce5b55844f7cd2b62c52a8fe3d790efbc0644140cda33547aa67",
		"03ae28a4100145a4c354338c727a54800dc540069fa2f5fd5d4a1c80b4a35a1762"
};
unsigned int pnSeed[] = //
		{ 0xa78a2879 };

class CMainParams: public CBaseParams {
public:
	CMainParams() {

// The message start string is designed to be unlikely to occur in normal data.
// The characters are rarely used upper ASCII, not valid as UTF-8, and produce
// a large 4-byte int at any alignment.
		pchMessageStart[0] = 0xff;
		pchMessageStart[1] = 0xfe;
		pchMessageStart[2] = 0x1d;
		pchMessageStart[3] = 0x20;
		vAlertPubKey =
				ParseHex(
						"04fc9702847840aaf195de8442ebecedf5b095cdbb9bc716bda9110971b28a49e0ead8564ff0db22209e0374782c093bb899692d524e9d6a6956e7c5ecbcd68284");
		nDefaultPort = 8668;
		nRPCPort = 8669;
		bnProofOfStakeLimit = CBigNum(~uint256(0) >> 10);        //00 3f ff ff
		nSubsidyHalvingInterval = 210000;

		// Build the genesis block. Note that the output of the genesis coinbase cannot
		// be spent as it did not originally exist in the database.
		//
		// CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
		//   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
		//     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
		//     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
		//   vMerkleTree: 4a5e1e
//		const char* pszTimestamp = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
//        CTransaction txNew;
//        txNew.vin.resize(1);
//        txNew.vout.resize(1);
//        txNew.vin[0].scriptSig = CScript() << 486604799 << CBigNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
//        txNew.vout[0].nValue = 50 * COIN;
//        txNew.vout[0].scriptPubKey = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
//        genesis.vtx.push_back(txNew);
		assert(CreateGenesisRewardTx(genesis.vptx));
		genesis.hashPrevBlock = 0;
		genesis.hashMerkleRoot = genesis.BuildMerkleTree();
		genesis.nVersion = 1;
		genesis.nTime = 1231006505;
		genesis.nBits = 0x1f3fffff;        //00 3f ff
		genesis.nNonce = 888;
		genesis.nHeight = 0;
		genesis.vSignature.clear();
		hashGenesisBlock = genesis.GetHash();
//		{
//			CBigNum bnTarget;
//			bnTarget.SetCompact(genesis.nBits);
//			cout << "main bnTarget:" << bnTarget.getuint256().GetHex() << endl;
//			cout << "main hashGenesisBlock:\r\n" << hashGenesisBlock.ToString() << endl;
//			cout << "main hashMerkleRoot:\r\n" << genesis.hashMerkleRoot.ToString() << endl;
//		}
//        cout << "hashGenesisBlock:" << HexStr(hashGenesisBlock) << endl;
//        cout << "hashMerkleRoot:" << genesis.hashMerkleRoot.GetHex() << endl;
//		assert(hashGenesisBlock == uint256("0x0d48e88dca01697d10e0fe8f1981f94db1f5e525d5a0e0acf22919af23daed60"));
//		assert(genesis.hashMerkleRoot == uint256("04b173fc873505d69f5f2a86aa8d7207abe7e0ffa63d786ff230f4a946f5a8255"));

        vSeeds.push_back(CDNSSeedData("soypay.org.cn", "seed_cn_0.dspay.org"));
        vSeeds.push_back(CDNSSeedData("soypay.org.us", "seed_us_0.dspay.org"));

		base58Prefixes[PUBKEY_ADDRESS] = {0};
		base58Prefixes[SCRIPT_ADDRESS] = {5};
		base58Prefixes[SECRET_KEY] = {128};
		base58Prefixes[EXT_PUBLIC_KEY] = {0x04,0x88,0xB2,0x1E};
		base58Prefixes[EXT_SECRET_KEY] = {0x04,0x88,0xAD,0xE4};

		// Convert the pnSeeds array into usable address objects.
		for (unsigned int i = 0; i < ARRAYLEN(pnSeed); i++) {
			// It'll only connect to one or two seed nodes because once it connects,
			// it'll get a pile of addresses with newer timestamps.
			// Seed nodes are given a random 'last seen time' of between one and two
			// weeks ago.
			const int64_t nOneWeek = 7 * 24 * 60 * 60;
			struct in_addr ip;
			memcpy(&ip, &pnSeed[i], sizeof(ip));
			CAddress addr(CService(ip, GetDefaultPort()));
			addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
			vFixedSeeds.push_back(addr);
		}
	}

	virtual const CBlock& GenesisBlock() const {
		return genesis;
	}
	virtual Network NetworkID() const {
		return CBaseParams::MAIN;
	}
	virtual bool InitalConfig() {
		return CBaseParams::InitalConfig();
	}
	virtual int GetBlockMaxNonce() const
	{
		return 1000;
	}
	virtual const vector<CAddress>& FixedSeeds() const {
		return vFixedSeeds;
	}

	virtual bool CreateGenesisRewardTx(vector<shared_ptr<CBaseTransaction> > &vRewardTx) {
		int length = sizeof(initPubKey) / (sizeof(initPubKey[0]));
		for (int i = 0; i < length; ++i) {
			shared_ptr<CRewardTransaction> pRewardTx = make_shared<CRewardTransaction>(ParseHex(initPubKey[i].c_str()), 10000000 * COIN, 0);
//		shared_ptr<CRewardTransaction>(
//					new CRewardTransaction(ParseHex(initPubKey[i].c_str()), 10000000 * COIN, 0));
			if (pRewardTx.get())
				vRewardTx.push_back(pRewardTx);
			else
				return false;
		}
		return true;
	}
protected:
	CBlock genesis;
	vector<CAddress> vFixedSeeds;
};
//static CMainParams mainParams;

//
// Testnet (v3)
//
class CTestNetParams: public CMainParams {
public:
	CTestNetParams() {
		// The message start string is designed to be unlikely to occur in normal data.
		// The characters are rarely used upper ASCII, not valid as UTF-8, and produce
		// a large 4-byte int at any alignment.
        pchMessageStart[0] = 0xfe;
        pchMessageStart[1] = 0x2d;
        pchMessageStart[2] = 0x1c;
        pchMessageStart[3] = 0x0d;
		vAlertPubKey =
				ParseHex(
						"04302390343f91cc401d56d68b123028bf52e5fca1939df127f63c646"
						"7cdf9c8e2c14b61104cf817d0b780da337893ecc4aaff1309e536162d"
						"abbdb45200ca2b0a");
		nDefaultPort = 18668;
		nRPCPort = 18669;
		strDataDir = "testnet3";

		// Modify the testnet genesis block so the timestamp is valid for a later start.
		genesis.nTime = 1296688602;
		genesis.nNonce = 888;
		hashGenesisBlock = genesis.GetHash();
//		{
//			CBigNum bnTarget;
//			bnTarget.SetCompact(genesis.nBits);
//			cout << "test bnTarget:" << bnTarget.getuint256().GetHex() << endl;
//			cout << "test hashGenesisBlock:\r\n" << hashGenesisBlock.ToString() << endl;
//			cout << "test hashMerkleRoot:\r\n" << genesis.hashMerkleRoot.ToString() << endl;
//		}
//		assert(hashGenesisBlock == uint256("0xeeae033352027ab2603e0d32c0585a0eb3b2e5f720d4de8eedec24050c66436f"));

//		vFixedSeeds.clear();
//		vSeeds.clear();
//		vSeeds.push_back(CDNSSeedData("Dacrs.petertodd.org", "testnet-seed.Dacrs.petertodd.org"));
//		vSeeds.push_back(CDNSSeedData("bluematt.me", "testnet-seed.bluematt.me"));

		base58Prefixes[PUBKEY_ADDRESS] = {111};
		base58Prefixes[SCRIPT_ADDRESS] = {196};
		base58Prefixes[SECRET_KEY] = {239};
		base58Prefixes[EXT_PUBLIC_KEY] = {0x04,0x35,0x87,0xCF};
		base58Prefixes[EXT_SECRET_KEY] = {0x04,0x35,0x83,0x94};
	}
	virtual Network NetworkID() const {return CBaseParams::TESTNET;}
	virtual bool InitalConfig()
	{
		CMainParams::InitalConfig();
		fServer = true;
		return true;
	}
	virtual int GetBlockMaxNonce() const
	{
		return 1000;
	}
};
//static CTestNetParams testNetParams;

//
// Regression test
//
class CRegTestParams: public CTestNetParams {
public:
	CRegTestParams() {
		pchMessageStart[0] = 0xfc;
		pchMessageStart[1] = 0x1d;
		pchMessageStart[2] = 0x2d;
		pchMessageStart[3] = 0x3d;
		nSubsidyHalvingInterval = 150;
		bnProofOfStakeLimit = CBigNum(~uint256(0) >> 8);        //00 00 ff ff
		genesis.nTime = 1296688602;
		genesis.nBits = 0x1fffffff;
		genesis.nNonce = 888;
		hashGenesisBlock = genesis.GetHash();
		nDefaultPort = 18666;
		nTargetSpacing = 20;
		nTargetTimespan = 30 * 20;
		strDataDir = "regtest";
//		{
//			CBigNum bnTarget;
//			bnTarget.SetCompact(genesis.nBits);
//			cout << "regtest bnTarget:" << bnTarget.getuint256().GetHex() << endl;
//			cout << "regtest hashGenesisBlock:\r\n" << hashGenesisBlock.ToString() << endl;
//			cout << "regtest hashMerkleRoot:\r\n" << genesis.hashMerkleRoot.ToString() << endl;
//		}
//		assert(hashGenesisBlock == uint256("0x891b4240b4005d26af25c634dcc886e5d3aaefb06da860e2a0ffc0132bd9df5a"));

		vSeeds.clear();  // Regtest mode doesn't have any DNS seeds.
	}

	virtual bool RequireRPCPassword() const {
		return false;
	}
	virtual Network NetworkID() const {
		return CBaseParams::REGTEST;
	}
	virtual bool InitalConfig() {
		CTestNetParams::InitalConfig();
		fServer = true;
		return true;
	}
};
//static CRegTestParams regTestParams;

//static CBaseParams *pCurrentParams = &mainParams;

//CBaseParams &Params() {
//	return *pCurrentParams;
//}

//void SelectParams(CBaseParams::Network network) {
//	switch (network) {
//	case CBaseParams::MAIN:
//		pCurrentParams = &mainParams;
//		break;
//	case CBaseParams::TESTNET:
//		pCurrentParams = &testNetParams;
//		break;
//	case CBaseParams::REGTEST:
//		pCurrentParams = &regTestParams;
//		break;
//	default:
//		assert(false && "Unimplemented network");
//		return;
//	}
//}

//bool SelectParamsFromCommandLine() {
//	bool fRegTest = GetBoolArg("-regtest", false);
//	bool fTestNet = GetBoolArg("-testnet", false);
//
//	if (fTestNet && fRegTest) {
//		return false;
//	}
//
//	if (fRegTest) {
//		SelectParams(CBaseParams::REGTEST);
//	} else if (fTestNet) {
//		SelectParams(CBaseParams::TESTNET);
//	} else {
//		SelectParams(CBaseParams::MAIN);
//	}
//	return true;
//}

/********************************************************************************/
const vector<string> &CBaseParams::GetMultiArgs(const string& strArg) {
	return m_mapMultiArgs[strArg];
}

string CBaseParams::GetArg(const string& strArg, const string& strDefault) {
	if (m_mapArgs.count(strArg))
		return m_mapArgs[strArg];
	return strDefault;
}

int64_t CBaseParams::GetArg(const string& strArg, int64_t nDefault) {
	if (m_mapArgs.count(strArg))
		return atoi64(m_mapArgs[strArg]);
	return nDefault;
}

bool CBaseParams::GetBoolArg(const string& strArg, bool fDefault) {
	if (m_mapArgs.count(strArg)) {
		if (m_mapArgs[strArg].empty())
			return true;
		return (atoi(m_mapArgs[strArg]) != 0);
	}
	return fDefault;
}

bool CBaseParams::SoftSetArg(const string& strArg, const string& strValue) {
	if (m_mapArgs.count(strArg))
		return false;
	m_mapArgs[strArg] = strValue;
	return true;
}

bool CBaseParams::SoftSetArgCover(const string& strArg, const string& strValue) {
	m_mapArgs[strArg] = strValue;
	return true;
}

bool CBaseParams::SoftSetBoolArg(const string& strArg, bool fValue) {
	if (fValue)
		return SoftSetArg(strArg, string("1"));
	else
		return SoftSetArg(strArg, string("0"));
}

bool CBaseParams::IsArgCount(const string& strArg) {
	if (m_mapArgs.count(strArg)) {
		return true;
	}
	return false;
}

const CBaseParams &SysCfg() {
	static shared_ptr<CBaseParams> pParams;

	if (pParams.get() == NULL) {
		bool fRegTest = CBaseParams::GetBoolArg("-regtest", false);
		bool fTestNet = CBaseParams::GetBoolArg("-testnet", false);
		if (fTestNet && fRegTest) {
			fprintf(stderr, "Error: Invalid combination of -regtest and -testnet.\n");
			assert(0);
		}

		if (fRegTest) {
			//LogPrint("spark", "In Reg Test Net\n");
			pParams = make_shared<CRegTestParams>();
		} else if (fTestNet) {
			//LogPrint("spark", "In Test Net\n");
			pParams = make_shared<CTestNetParams>();
		} else {
			//LogPrint("spark", "In Main Net\n");
			pParams = make_shared<CMainParams>();
		}

	}
	assert(pParams != NULL);
	return *pParams.get();
}

//write for test code
const CBaseParams &SysParamsMain() {
	static std::shared_ptr<CBaseParams> pParams;
	pParams = make_shared<CMainParams>();
	assert(pParams != NULL);
	return *pParams.get();
}

//write for test code
const CBaseParams &SysParamsTest() {
	static std::shared_ptr<CBaseParams> pParams;
	pParams = make_shared<CTestNetParams>();
	assert(pParams != NULL);
	return *pParams.get();
}

//write for test code
const CBaseParams &SysParamsReg() {
	static std::shared_ptr<CBaseParams> pParams;
	pParams = make_shared<CRegTestParams>();
	assert(pParams != NULL);
	return *pParams.get();
}

static void InterpretNegativeSetting(string name, map<string, string>& mapSettingsRet) {
	// interpret -nofoo as -foo=0 (and -nofoo=0 as -foo=1) as long as -foo not set
	if (name.find("-no") == 0) {
		string positive("-");
		positive.append(name.begin() + 3, name.end());
		if (mapSettingsRet.count(positive) == 0) {
			bool value = !SysCfg().GetBoolArg(name, false);
			mapSettingsRet[positive] = (value ? "1" : "0");
		}
	}
}

void CBaseParams::ParseParameters(int argc, const char* const argv[]) {
	m_mapArgs.clear();
	m_mapMultiArgs.clear();
	for (int i = 1; i < argc; i++) {
		string str(argv[i]);
		string strValue;
		size_t is_index = str.find('=');
		if (is_index != string::npos) {
			strValue = str.substr(is_index + 1);
			str = str.substr(0, is_index);
		}
#ifdef WIN32
		boost::to_lower(str);
		if (boost::algorithm::starts_with(str, "/"))
			str = "-" + str.substr(1);
#endif
		if (str[0] != '-')
			break;

		m_mapArgs[str] = strValue;
		m_mapMultiArgs[str].push_back(strValue);
	}

	// New 0.6 features:
//	BOOST_FOREACH(const PAIRTYPE(string,string)& entry, m_mapArgs) {
	for (auto& entry : m_mapArgs) {
		string name = entry.first;

		//  interpret --foo as -foo (as long as both are not set)
		if (name.find("--") == 0) {
			string singleDash(name.begin() + 1, name.end());
			if (m_mapArgs.count(singleDash) == 0)
				m_mapArgs[singleDash] = entry.second;
			name = singleDash;
		}

		// interpret -nofoo as -foo=0 (and -nofoo=0 as -foo=1) as long as -foo not set
		InterpretNegativeSetting(name, m_mapArgs);
	}
#if 0
	for(const auto& tmp:m_mapArgs) {
		printf("key:%s - value:%s\n", tmp.first.c_str(), tmp.second.c_str());
	}
#endif
}

bool CBaseParams::IntialParams(int argc, const char* const argv[]) {
	ParseParameters(argc, argv);
	if (!boost::filesystem::is_directory(GetDataDir(false))) {
		fprintf(stderr, "Error: Specified data directory \"%s\" does not exist.\n", CBaseParams::m_mapArgs["-datadir"].c_str());
		return false;
	}
	try {
		ReadConfigFile(CBaseParams::m_mapArgs, CBaseParams::m_mapMultiArgs);
	} catch (exception &e) {
		fprintf(stderr, "Error reading configuration file: %s\n", e.what());
		return false;
	}
	return true;
}

int64_t CBaseParams::GetTxFee() const{
     return paytxfee;
}
int64_t CBaseParams::SetDeflautTxFee(int64_t fee)const{
	paytxfee = fee;

	return fee;
}

CBaseParams::CBaseParams() {
	fImporting = false;
	fReindex = false;
	fBenchmark = false;
	fTxIndex = false;
	nIntervalPos = 1;
	nTxCacheHeight = 500;
	nTimeBestReceived = 0;
	nScriptCheckThreads = 0;
	nViewCacheSize = 2000000;
	nTargetSpacing = 60;
	nTargetTimespan = 30 * 60;
	nMaxCoinDay = 30 * 24 * 60 * 60;
	nSubsidyHalvingInterval = 0;
	paytxfee = 200000;
	nDefaultPort = 0;
	fPrintToConsole= 0;
	fPrintToToFile = 0;
	fLogTimestamps = 0;
	fLogPrintFileLine = 0;
	fDebug = 0;
	fDebugAll= 0 ;
	fServer = 0 ;
	fServer = 0;
	nRPCPort = 0;


}
/********************************************************************************/

