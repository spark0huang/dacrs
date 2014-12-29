#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "SysTestBase.h"
using namespace std;
using namespace boost;

class CSysRegisterAccTest:public SysTestBase {
public:
	bool GetKeyId(string const &addr,CKeyID &KeyId) {
		if (!CRegID::GetKeyID(addr, KeyId)) {
			KeyId=CKeyID(addr);
			if (KeyId.IsEmpty())
			return false;
		}
		return true;
	};

	bool RegisterAccount(const string& strAddr, uint64_t nFee,string& strTxHash,int nHeight = 0,bool bSign = true) {
		CKeyID keyid;
		if (!GetKeyId(strAddr, keyid)) {
			//return false;
		}


		CPubKey pubkey;
		if (!pwalletMain->GetPubKey(keyid, pubkey)) {
			//return false;
		}

		Cregistaccounttx rtx;
		CNullID nullId;
		rtx.minerId = nullId;

		rtx.userId = pubkey;
		rtx.llFees = nFee;
		rtx.nValidHeight = (0 != nHeight?nHeight:chainActive.Tip()->nHeight);

		if (bSign) {
			if (!pwalletMain->Sign(keyid, rtx.SignatureHash(), rtx.signature)) {
				//return false;
			}
		}


		std::tuple<bool,string> ret = pwalletMain->CommitTransaction((CBaseTransaction *) &rtx);
		if (std::get<0>(ret)) {
			strTxHash = std::get<1>(ret);
		} else {
			strTxHash = rtx.GetHash().ToString();
		}

		return std::get<0>(ret);
	}

	bool SendMoney(const string& strRegAddr, const string& strDestAddr, uint64_t nMoney, uint64_t nFee=0) {
		CKeyID keyid;
		if (!GetKeyId(strDestAddr, keyid))
			return false;
		std::tuple<bool, string> ret = pwalletMain->SendMoney(strRegAddr, CUserID(keyid), nMoney, nFee);
		return std::get<0>(ret);
	}

	bool GetAccountInfo(const string& strAddr,CAccount& account) {
		CKeyID keyid;
		if (!GetKeyId(strAddr, keyid)) {
			return false;
		}

		CUserID userId = keyid;

		LOCK(cs_main);
		CAccountViewCache accView(*pAccountViewTip, true);
		if (!accView.GetAccount(userId, account)) {
			return false;
		}

		return true;
	}

	int GetBlockHeight() {
		return (int) chainActive.Height();
	}

	bool GenerateOneBlock() {
		int nOldBlockHeight = GetBlockHeight();
		if (!SysTestBase::GenerateOneBlock()) {
			return false;
		}

		int nNewBlockHeight = GetBlockHeight();
		if (nNewBlockHeight != nOldBlockHeight + 1) {
			return false;
		}

		return true;
	}

	bool GetRegID(string& strAddr,CRegID& regID) {
		CAccount account;
		if (!GetAccountInfo(strAddr,account)) {
			return false;
		}

		regID = account.regID;
		return true;
	}

	bool GetNewAddress(bool IsForMiner,string& strNewKeyID)
	{
	    CKey  mCkey;
	    mCkey.MakeNewKey();

	    CKey  Minter;
	    Minter.MakeNewKey();

	    CPubKey newKey = mCkey.GetPubKey();
	    CKeyID keyID = newKey.GetKeyID();

		if (IsForMiner) {
			if (!pwalletMain->AddKey(mCkey, Minter))
				return false;
		}
		else if (!pwalletMain->AddKey(mCkey)) {
			return false;
		}

		strNewKeyID = keyID.ToAddress();
		return true;
	}

	bool IsTxInMemorypool(const uint256& txHash) {
		for (const auto& entry : mempool.mapTx) {
			if (entry.first == txHash)
				return true;
		}

		return false;
	}

	bool IsTxUnConfirmdInWallet(const uint256& txHash) {
			for (const auto &item : pwalletMain->UnConfirmTx) {
				if (txHash == item.first) {
					return true;
				}
			}
			return false;
		}

	bool GenerateBlock(const string& strKeyID) {
		int nOldBlockHeight = GetBlockHeight();

		CKeyID keyID(strKeyID);
		uint256 hash = CreateBlockWithAppointedAddr(keyID);

		int nNewBlockHeight = GetBlockHeight();
		if (nNewBlockHeight != nOldBlockHeight + 1) {
			return false;
		}
		return 0 != hash;
	}

	bool GetBlockHash(int nHeight, uint256& blockHash) {
		if (nHeight < 0 || nHeight > chainActive.Height())
			return false;

		CBlockIndex* pblockindex = chainActive[nHeight];
		blockHash = pblockindex->GetBlockHash();
		return true;
	}

	bool IsTxInTipBlock(const uint256& txHash) {
		CBlockIndex* pindex = chainActive.Tip();
		CBlock block;
		if (!ReadBlockFromDisk(block, pindex))
			return false;

		block.BuildMerkleTree();
		std::tuple<bool, int> ret = block.GetTxIndex(txHash);
		if (!std::get<0>(ret)) {
			return false;
		}

		return true;
	}

	bool GetTxOperateLog(const uint256& txHash, vector<CAccountOperLog>& vLog) {
		if (!GetTxOperLog(txHash, vLog))
			return false;

		return true;
	}


};

BOOST_FIXTURE_TEST_SUITE(sysregisteracc_test,CSysRegisterAccTest)
BOOST_FIXTURE_TEST_CASE(rpc_test,CSysRegisterAccTest)
{
	ResetEnv();
	//ת��
	string strRegAddr = "mo51PMpnadiFx5JcZaeUdWBa4ngLBVgoGz";
	string strSrcRegID = "000000000400";
	uint64_t nMoney = 100000;
	BOOST_CHECK(SendMoney(strSrcRegID,strRegAddr,nMoney));
	BOOST_CHECK(GenerateOneBlock());

	//ȷ��ת�˳ɹ�
	uint64_t nFreeMoney = GetFreeMoney(strRegAddr);
	BOOST_CHECK(nFreeMoney == nMoney);

	//ʹ��һ��û��ע�����keyID��ַ���ԣ������ܹ�ע�ᣬ��getaccountinfo ���regid�Ƿ����
	string strTxHash;
	CRegID regID;
	BOOST_CHECK(RegisterAccount(strRegAddr,10000,strTxHash));
	BOOST_CHECK(GenerateOneBlock());
	BOOST_CHECK(GetRegID(strRegAddr,regID));
	BOOST_CHECK(false == regID.IsEmpty());

	//ʹ��һ��ע�����keyID��ַ���ԣ����Ƿ��ܹ�ע��ɹ�
	string strResigterd("msdDQ1SXNmknrLuTDivmJiavu5J9VyX9fV");
	BOOST_CHECK(!RegisterAccount(strRegAddr,10000,strTxHash));

	//��getnewaddress��ȡһ��û�������ڿӵĹ�Կ��ַ��ע���˻����һ��������true�����˵�ַ�Ƿ��ܹ�ע��ɹ�
	string strNewKeyID;
	BOOST_CHECK(GetNewAddress(true,strNewKeyID));

	//��һ��ע��ĵ�ַ��һ��δע��ĵ�ַ����һ�����ף����Ƿ��ܹ����ͳɹ�
	string strUnRegister("mydRNvqewpZt9tyNtBSmBCrKr1NTiii5JH");
	vector<unsigned char> vRegID = regID.GetVec6();
	nMoney = nMoney/10;
	BOOST_CHECK(SendMoney(HexStr(vRegID),strUnRegister,nMoney, 10000));
	BOOST_CHECK(GenerateOneBlock());

	//ȷ��ת�˳ɹ�
	nFreeMoney = GetFreeMoney(strUnRegister);
	BOOST_CHECK(nFreeMoney == nMoney);
}

BOOST_FIXTURE_TEST_CASE(sysonly_test,CSysRegisterAccTest)
 {
	ResetEnv();
	string strTxHash;
	CRegID regID;
	string strRegAddr1("mo51PMpnadiFx5JcZaeUdWBa4ngLBVgoGz");
	string strRegAddr2("mydRNvqewpZt9tyNtBSmBCrKr1NTiii5JH");
	int64_t nFee = 10000;
	vector<string> vFailedTxHash;

	//��Ч�ĸ߶�
	int nInValidHeight = 1000;
	BOOST_CHECK(!RegisterAccount(strRegAddr1, nFee, strTxHash,nInValidHeight));
	vFailedTxHash.push_back(strTxHash);
	nInValidHeight = 100;

	//�޷���ȡ���˺ŵ�ַ
	string strInvalidAddr("fjsofeoifdsfdsfdsafafafafafafafa");
	BOOST_CHECK(!RegisterAccount(strInvalidAddr, nFee, strTxHash,nInValidHeight));
	vFailedTxHash.push_back(strTxHash);

	//��ǩ��
	BOOST_CHECK(!RegisterAccount(strRegAddr1, nFee, strTxHash,nInValidHeight,false));
	vFailedTxHash.push_back(strTxHash);

	//�����ѳ������ֵ
	BOOST_CHECK(!RegisterAccount(strRegAddr1, nFee+MAX_MONEY, strTxHash,nInValidHeight));
	vFailedTxHash.push_back(strTxHash);

	//�ظ�ע��ĵ�ַ
	string strReRegisrerAddr("mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc");
	BOOST_CHECK(!RegisterAccount(strReRegisrerAddr, nFee, strTxHash,nInValidHeight));
	vFailedTxHash.push_back(strTxHash);

	//�˻���û������δע���ַ
	BOOST_CHECK(!RegisterAccount(strRegAddr1, nFee, strTxHash,nInValidHeight));
	vFailedTxHash.push_back(strTxHash);

	//���ʧ�ܵĽ����Ƿ���memorypool��
	for (const auto& item : vFailedTxHash) {
		uint256 txHash(item);
		BOOST_CHECK(!IsTxInMemorypool(txHash));
		BOOST_CHECK(!IsTxUnConfirmdInWallet(txHash));
	}

	//��û�������˻�ת��
	string strSrcRegID = "000000000400";
	uint64_t nMoney = 100000;
	BOOST_CHECK(SendMoney(strSrcRegID, strRegAddr1, nMoney));
	BOOST_CHECK(GenerateOneBlock());

	//ȷ��ת�˳ɹ�
	uint64_t nFreeMoney = GetFreeMoney(strRegAddr1);
	BOOST_CHECK(nFreeMoney == nMoney);

	//�ٴμ��ʧ�ܵĽ����Ƿ���memorypool��
	for (const auto& item : vFailedTxHash) {
		uint256 txHash(item);
		BOOST_CHECK(!IsTxInMemorypool(txHash));
		BOOST_CHECK(!IsTxUnConfirmdInWallet(txHash));
	}


	string strSpecial;
	BOOST_CHECK(RegisterAccount(strRegAddr1, nFee, strSpecial,nInValidHeight));
	BOOST_CHECK(IsTxInMemorypool(uint256(strSpecial)));
	BOOST_CHECK(IsTxUnConfirmdInWallet(uint256(strSpecial)));

	//�����Ѿ���memorypool��
	BOOST_CHECK(!RegisterAccount(strRegAddr1, nFee, strTxHash,nInValidHeight));
	BOOST_CHECK(!IsTxInMemorypool(uint256(strTxHash)));
	BOOST_CHECK(!IsTxUnConfirmdInWallet(uint256(strTxHash)));

	BOOST_CHECK(GenerateOneBlock());

	//ȷ��ע��ɹ��Ľ�����tip��
	BOOST_CHECK(IsTxInTipBlock(uint256(strSpecial)));

	vector<CAccountOperLog> vLog;
	BOOST_CHECK(GetTxOperateLog(uint256(strSpecial),vLog));

	//�����־��¼�Ƿ���ȷ
	BOOST_CHECK(1 == vLog.size() && 1 == vLog[0].vOperFund.size() && 1 == vLog[0].vOperFund[0].vFund.size());
	BOOST_CHECK(strRegAddr1 == vLog[0].keyID.ToAddress());
	BOOST_CHECK(vLog[0].vOperFund[0].operType == MINUS_FREE && vLog[0].vOperFund[0].vFund[0].value == nFee);
}

BOOST_AUTO_TEST_SUITE_END()






