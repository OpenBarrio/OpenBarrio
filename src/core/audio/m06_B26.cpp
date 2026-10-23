#include "ts3/audio/m06_B26.h"
namespace ts3::audio::b26impl {
namespace {
Node* Sentinel(Tree* s) { return reinterpret_cast<Node*>(&s->header); }
Tree* Init(Tree* s,uint32_t bytes,uint32_t stride) {
 void* temporary=nullptr;
 CallGameMethod<void,0x4e07a0>(&temporary,static_cast<void*>(nullptr),bytes,stride,4,0);
 s->header.left=nullptr;s->header.right=nullptr;s->header.parent=nullptr;s->header.color=0;
 // The original clears all four header words before initializing the pool.
 s->header.pad[0]=s->header.pad[1]=s->header.pad[2]=0;s->count=0;s->free=nullptr;
 CallGameMethod<void,0x4e07a0>(&s->free,temporary,bytes,stride,4,0);
 s->header.left=Sentinel(s);s->header.right=Sentinel(s);s->header.parent=nullptr;s->header.color=0;s->count=0;
 CallGameMethod<void,0x4e07a0>(&s->free,static_cast<void*>(s->storage),bytes,stride,4,0);
 return s;
}
Result* FindInsert(Tree* s,Result* out,const Value* value,bool flagged) {
 Node* p=Sentinel(s);Node* n=s->header.parent;bool less=true;
 while(n){p=n;less=value->key<n->key;n=less?n->right:n->left;}
 Node* candidate=p;
 bool insert=false;
 if(less){if(candidate==s->header.right)insert=true;else candidate=CallGameFunction<Node*,0x4e0b60>(candidate);}
 if(!insert)insert=candidate->key<value->key;
 if(insert){Node* added=nullptr;
  if(flagged)InsertFlagged(s,&added,p,value,0);
  else CallGameMethod<Node**,0x871610>(s,&added,p,value,0);
  out->node=added;out->inserted=1;
 }else{out->node=candidate;out->inserted=0;}
 return out;
}
void Free(void* p) { CallGameFunction<void,0x4e3f70>(p); }
}
Node** InsertFlagged(Tree* s,Node** out,Node* parent,const Value* value,uint32_t force) {
 uint32_t side=!(uint8_t(force)) && parent!=Sentinel(s) && value->key>=parent->key;
 Node* n=s->free;if(n)s->free=n->left;
 n->key=value->key;n->object=value->object;
 if(n->object)CallGameVirtual<void,0>(n->object);
 n->flag=value->flag;
 CallGameFunction<void,0x4e0c40>(n,parent,Sentinel(s),side);
 ++s->count;*out=n;return out;
}
Node* CloneNode(Tree* s,const Node* source,Node* parent) {
 Node* n=s->free;if(n)s->free=n->left;
 n->key=source->key;n->object=source->object;
 if(n->object)CallGameVirtual<void,0>(n->object);
 n->left=nullptr;n->right=nullptr;n->parent=parent;n->color=source->color;
 return n;
}
Tree* Construct(Tree* s) { return Init(s,120,24); }
Result* Unique(Tree* s,Result* out,const Value* value,uint32_t) { return FindInsert(s,out,value,false); }
Result* UniqueFlagged(Tree* s,Result* out,const Value* value,uint32_t) { return FindInsert(s,out,value,true); }
Node* CloneTree(Tree* s,const Node* source,Node* parent) {
 Node* root=CloneNode(s,source,parent);Node* dest=root;
 if(source->left)dest->left=CloneTree(s,source->left,dest);
 for(source=source->right;source;source=source->right){
  Node* n=CloneNode(s,source,dest);dest->right=n;
  if(source->left)n->left=CloneTree(s,source->left,n);
  dest=n;
 }
 return root;
}
uintptr_t DestroySound(Sound* s) {
#if defined(_M_IX86)
 s->primary=reinterpret_cast<void*>(0x102a230);s->secondary=reinterpret_cast<void*>(0x103a910);s->third=reinterpret_cast<void*>(0x102a22c);
#else
 AbortNotRewritten("m06_B26 sound game vtables");
#endif
 for(unsigned i=0;i<3;++i){Curve* c=s->curves[i];if(c){if(c->begin && c->begin!=c->storage)Free(c->begin);Free(c);s->curves[i]=nullptr;}}
 auto span=static_cast<intptr_t>((reinterpret_cast<uintptr_t>(s->capacity)-reinterpret_cast<uintptr_t>(s->begin))&~uintptr_t(1));
 if(span>2 && s->begin)Free(s->begin);
 return CallGameMethod<uintptr_t,0x8b0140>(s);
}
Tree* ConstructFlagged(Tree* s) { return Init(s,224,28); }
Result* Insert(Tree* s,Result* out,const Value* value) {Unique(s,out,value,0);return out;}
Result* InsertWithFlag(Tree* s,Result* out,const Value* value) {UniqueFlagged(s,out,value,0);return out;}
Tree* Assign(Tree* s,const Tree* source) {
 if(s==source)return s;
 CallGameMethod<void,0x8715d0>(s,s->header.parent);
 s->header.left=Sentinel(s);s->header.right=Sentinel(s);s->header.parent=nullptr;s->header.color=0;s->count=0;
 if(source->header.parent){
  Node* root=CloneTree(s,source->header.parent,Sentinel(s));s->header.parent=root;
  Node* n=root;while(n->left)n=n->left;s->header.left=n;
  n=root;while(n->right)n=n->right;s->header.right=n;s->count=source->count;
 }
 return s;
}
Sound* DeleteSound(Sound* s,uint32_t flags) {DestroySound(s);if(flags&1)Free(s);return s;}
Tree* CopyConstruct(Tree* s,const Tree* source) {Construct(s);Assign(s,source);return s;}
BucketIterator* Erase(Hash* s,BucketIterator* out,BucketNode* node,BucketNode** bucket) {
 out->bucket=bucket;out->node=node->next;
 while(!out->node)out->node=*++out->bucket;
 BucketNode* p=*bucket;
 if(p==node)*bucket=p->next;
 else {while(p->next!=node)p=p->next;p->next=node->next;}
 CallGameMethod<void,0x8715d0>(&node->tree,node->tree.header.parent);
 Free(node);--s->count;return out;
}
}
