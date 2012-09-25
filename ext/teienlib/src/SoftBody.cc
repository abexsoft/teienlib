#include "SoftBody.h"

#include "MeshStrider.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "btSoftBodyHelpers.h"

btSoftBody*
SoftBody::createFromMesh(Ogre::Mesh* mesh, btSoftBodyWorldInfo* worldInfo)
{

	std::cout << __PRETTY_FUNCTION__ << std::endl;

	MeshStrider meshStrider(mesh);


	std::cout << meshStrider.getNumSubParts() << std::endl;

	const unsigned char *vertexbase;
	int numverts;
	PHY_ScalarType type;
	int stride;

	const unsigned char *indexbase;
	int indexstride;
	int numfaces;
	PHY_ScalarType indicestype;

	btSoftBody *softBody;

	meshStrider.getLockedReadOnlyVertexIndexBase(&vertexbase, numverts, type, stride,
						     &indexbase, indexstride, numfaces, indicestype,
						     0);

	std::cout << "NumVerts: " << numverts << ", " << stride << std::endl;
	std::cout << "NumFaces: " << numfaces << ", " << indexstride << std::endl;

	
	int* triangles = new int(numfaces * 3);
	for (int i = 0; i < numfaces * 3; i++) {
		triangles[i] = ((unsigned short*)indexbase)[i];
	}


	// *** glibc detected *** ruby: double free or corruption (fasttop): 0x09445008 ***
	//softBody = btSoftBodyHelpers::CreateFromTriMesh(*worldInfo, (btScalar *)vertexbase, triangles, numfaces);
	std::cout << "done" << std::endl;

	meshStrider.unLockReadOnlyVertexBase(0);

	return softBody;

}


void
SoftBody::updateOgreMesh(Ogre::Mesh* mesh, btSoftBody* softBody)
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;

/*
     Ogre::Node *ogreNode = mEntity->getParentNode();
      
      //printf("updateOgreMesh %d %s %s\n", internalId, mEntity->getName().c_str(), ogreNode->getName().c_str());
      
      MeshPtr mesh = mEntity->getMesh();
      Mesh::SubMeshIterator subMeshI = mesh->getSubMeshIterator();
      SubMesh* subMesh = NULL;
      
      VertexData* vData = NULL;
      VertexDeclaration* vDeclaration = NULL;
      const VertexElement* vPosElement = NULL;
      
      bool isSharedVerticesAdded = false;
      unsigned short bufferIndex = 0;
      HardwareVertexBufferSharedPtr vBuffer;
      
      // Can not do arithmetic operations on void*
      unsigned char* lockedMem = NULL;
      float* vPosition;
      
      btSoftBody::tNodeArray& btNodes = mSoftBody->m_nodes;
      //printf("Bullet nodes size %d\n", btNodes.size());
      
      int ogreVertexIdx = 0;
                btVector3 btPosOffset;
      
      while (subMeshI.hasMoreElements()) {
         subMesh = subMeshI.getNext();
         
         if (subMesh->useSharedVertices) {
            
            if (isSharedVerticesAdded) {
               continue;
            }
            
            vData = mesh->sharedVertexData;
            
            // We need to add shared vertices only once
            isSharedVerticesAdded = true;
         } else  {
            vData = subMesh->vertexData;
         }
         
         vDeclaration = vData->vertexDeclaration;
         vPosElement = vDeclaration->findElementBySemantic(VES_POSITION);
         
         bufferIndex = vPosElement->getSource();
         vBuffer = vData->vertexBufferBinding->getBuffer(bufferIndex);
         
         // Lock the buffer before reading from it
         lockedMem = static_cast<unsigned char*>(vBuffer->lock(HardwareBuffer::HBL_DISCARD));
         


         // Read each vertex
         for (unsigned int i = 0; i < vData->vertexCount; ++i) {
            vPosElement->baseVertexPointerToElement(lockedMem, &vPosition);
            
            int idx = getBulletIndex(ogreVertexIdx);
                                const btVector3 &btPos = btNodes[idx].m_x;
                                if (ogreVertexIdx == 0) {
                                    btPosOffset = btPos;
                                }

                                *vPosition++ = btPos.x() - btPosOffset.x();
                                *vPosition++ = btPos.y() - btPosOffset.y();
                                *vPosition = btPos.z() - btPosOffset.z();
            
            // Point to the next vertex
            lockedMem += vBuffer->getVertexSize();
            
            ogreVertexIdx++;
         }
         
         vBuffer->unlock();
      }

                btTransform transform = mSoftBody->getWorldTransform();
                btQuaternion rot = transform.getRotation();
                ogreNode->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
                btVector3 pos = transform.getOrigin();
                ogreNode->setPosition(pos.x() + btPosOffset.x(), pos.y() + btPosOffset.y(), pos.z() + btPosOffset.z());
      
   }
*/
}
