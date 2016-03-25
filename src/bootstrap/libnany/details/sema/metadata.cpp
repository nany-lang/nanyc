#include "details/sema/metadata.h"
#include <yuni/thread/mutex.h>
#include "details/grammar/nany.h"

using namespace Yuni;




namespace Nany
{
namespace Sema
{

	void callbackMetadataRelease(void* ptr)
	{
		assert(ptr != nullptr and "invalid metadata pointer");
		Metadata::release((Metadata*) ptr);
	}


	void* callbackMetadataClone(void* ptr)
	{
		assert(ptr != nullptr and "invalid metadata pointer");
		return Metadata::clone(*((const Metadata*) ptr));
	}


	void Metadata::initialize()
	{
		// initializing handlers
		static Yuni::Mutex mutex;
		mutex.lock();
		AST::Node::metadataRelease = callbackMetadataRelease;
		AST::Node::metadataClone   = callbackMetadataClone;
		mutex.unlock();
	}


	inline Metadata::Metadata(AST::Node* originalNode)
	{
		this->originalNode = originalNode;
	}


	void Metadata::reset()
	{
		fromASTTransformation = true;
		parent = nullptr;
		originalNode = nullptr;
	}


	Metadata* Metadata::create(AST::Node* originalNode)
	{
		return new Metadata{originalNode};
	}


	Metadata* Metadata::clone(const Metadata& rhs)
	{
		return new Metadata{rhs};
	}


	void Metadata::release(Metadata* ptr)
	{
		delete ptr;
	}





} // namespace Sema
} // namespace Nany
