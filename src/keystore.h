// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_KEYSTORE_H
#define DACRS_KEYSTORE_H

#include "key.h"
#include "sync.h"
#include <set>

#include <boost/signals2/signal.hpp>


/** A virtual base class for key stores */
class CKeyStore
{
protected:
    mutable CCriticalSection cs_KeyStore;

public:
    virtual ~CKeyStore() {}

    // Add a key to the store.
    virtual bool AddKeyPubKey(const CKey &key, const CPubKey &pubkey) =0;
    virtual bool AddKey(const CKey &key);

    // Check whether a key corresponding to a given address is present in the store.
    virtual bool HaveKey(const CKeyID &address) const =0;
    virtual bool GetKey(const CKeyID &address, CKey& keyOut) const =0;
    virtual void GetKeys(set<CKeyID> &setAddress) const =0;
    virtual bool GetPubKey(const CKeyID &address, CPubKey& vchPubKeyOut) const;

    // Support for BIP 0013 : see https://github.com/bitcoin/bips/blob/master/bip-0013.mediawiki
//    virtual bool AddCScript(const CScript& redeemScript) =0;
//    virtual bool HaveCScript(const CScriptID &hash) const =0;
//    virtual bool GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const =0;
};

typedef map<CKeyID, CKey> KeyMap;
//typedef map<CScriptID, CScript > ScriptMap;

/** Basic key store, that keeps keys in an address->secret map */
class CBasicKeyStore : public CKeyStore
{
protected:
    KeyMap mapKeys;
  //  ScriptMap mapScripts;

public:
    bool AddKeyPubKey(const CKey& key, const CPubKey &pubkey);
    bool HaveKey(const CKeyID &address) const
    {
        bool result;
        {
            LOCK(cs_KeyStore);
            result = (mapKeys.count(address) > 0);
        }
        return result;
    }
    void GetKeys(set<CKeyID> &setAddress) const
    {
        setAddress.clear();
        {
            LOCK(cs_KeyStore);
            KeyMap::const_iterator mi = mapKeys.begin();
            while (mi != mapKeys.end())
            {
                setAddress.insert((*mi).first);
                mi++;
            }
        }
    }
    bool GetKey(const CKeyID &address, CKey &keyOut) const
    {
        {
            LOCK(cs_KeyStore);
            KeyMap::const_iterator mi = mapKeys.find(address);
            if (mi != mapKeys.end())
            {
                keyOut = mi->second;
                return true;
            }
        }
        return false;
    }
//    virtual bool AddCScript(const CScript& redeemScript);
//    virtual bool HaveCScript(const CScriptID &hash) const;
//    virtual bool GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const;
};

typedef vector<unsigned char, secure_allocator<unsigned char> > CKeyingMaterial;
typedef map<CKeyID, pair<CPubKey, vector<unsigned char> > > CryptedKeyMap;



#endif
