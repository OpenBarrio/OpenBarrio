// Unit clu_617200_CAS: TS3W.exe 0x00617200 (bucket-chain slot search, 93 bytes) and
// 0x00617540 (value-slot find, 57 bytes). Both are read-only walks over the open-chained
// hash table modelled in the unit's header; 0x00617540 is the only in-unit caller of
// 0x00617200 (the other 12 callers belong to other units and go through the bridge).
#include "ts3/cas/clu_617200_CAS.h"

namespace ts3::cas {

CasSlotPair* CasLookupTable::FindSlot(CasSlotPair* out, const uint32_t* key)
{
    // The original (0x00617200, thiscall ret 8) reads *key once into esi and keeps it
    // for both the divisor remainder and the chain compares; likewise here.
    const uint32_t k = *key;
    const uint32_t n = count;          // this+0x08, read before the buckets pointer
    CasLookupNode** const bucketsList = buckets;  // this+0x04
    CasLookupNode** const slot = &bucketsList[k % n];
    CasLookupNode* node = *slot;
    // Chain walk: key compared bit-exactly (dword), next at node+0x28. EA: on n == 0
    // the original divides by zero (#DE) - the game never builds an empty table, and
    // the hardware trap is reproduced by the plain unsigned division here.
    while (node != nullptr && node->key != k)
        node = node->next;

    CasSlotPair pair;
    if (node != nullptr) {
        pair.node = node;
        pair.bucket = slot;            // the found node's own chain head
    } else {
        pair.node = bucketsList[n];     // the sentinel at the extra slot `count`
        pair.bucket = &bucketsList[n];  // (the insert side's "not found" marker)
    }
    *out = pair;
    return out;  // the original returns a2 in eax
}

void* CasLookupOwner::FindValueSlot(uint32_t key)
{
    // The original (0x00617540, thiscall ret 4) takes the stack argument's address and
    // hands it to the 0x00617200 lookup on the +0x20 sub-object; the local copy here is
    // equivalent (the lookup only ever reads *key).
    CasSlotPair pair;
    table.FindSlot(&pair, &key);
    // EA: "not found" is detected by comparing the returned NODE against the sentinel
    // buckets[count] (not by a null check) - a node that somehow aliases the sentinel
    // address would read as "not found" here exactly as in the original.
    const bool missing = pair.node == table.buckets[table.count];
    return missing ? nullptr : &pair.node->value04;  // the original returns node + 4
}

}  // namespace ts3::cas
