// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <PRManProcedural.h>

#include <pystring/pystring.h>
namespace SPI_pystring = GEOLIB3_PYSTRING_NAMESPACE;

#include <string>
#include <map>
#include <list>
#include <algorithm>
#include <iostream>
#include <sys/time.h>
#include <memory>

#include <OpenEXR/ImathMatrix.h>
SPI_GEOLIB_IMATH_NAMESPACE
using namespace Imath;
#include <ErrorReporting.h>

#include <boost/thread.hpp>

#include <FnGeolibServices/FnXFormUtil.h>

using namespace FnKat;

/////////////////////////// TIMING UTILS //////////////////////////////////////////

namespace
{

    // ***********************************************************************
    // Deprecation Warning
    // These classes should no longer be used and will soon be deprecated.
    //
    // Consider using Renderman Rix functionality rather than these.
    class SimpleTimer
    {
        public:
            SimpleTimer(std::string name):
                name_(name),
                running_(false),
                total_(0),
                print_(false),
                timerMutex_() {}
            
            ~SimpleTimer()
            {
                if(print_)
                {
                    if(total_ < 60.0)
                    {
                        std::cout << name_ << " " << total_ << " s" << std::endl;
                    }
                    else
                    {
                        std::cout << name_ << " " << total_ / 60.0 << " min" << std::endl;
                    }
                }
            }
            
            void setPrint(bool print)
            {
                print_ = print;
            }
            
            void start()
            {
                boost::recursive_mutex::scoped_lock lock(timerMutex_);
                timeval t;
                gettimeofday(&t, 0);
                start_ = (double) t.tv_sec + (double) t.tv_usec / 1000000.0;
                running_ = true;
            }
            
            void stop()
            {
                boost::recursive_mutex::scoped_lock lock(timerMutex_);
                if(!running_)
                    return;
                
                timeval t;
                gettimeofday(&t, 0);
                double end =  (double) t.tv_sec + (double) t.tv_usec / 1000000.0;
                total_ += end - start_;
            }
        
        private:
            SimpleTimer (const SimpleTimer&);
            SimpleTimer& operator= (const SimpleTimer&);
            std::string name_;
            bool running_;
            double total_; // total is in seconds
            double start_;
            bool print_;
            boost::recursive_mutex timerMutex_;
    };
    
    
    
    class SimpleCounter
    {
        public:
            SimpleCounter(std::string name):
                name_(name),
                total_(0),
                print_(false) {}
            
            ~SimpleCounter()
            {
                if(print_)
                {
                    {
                        std::cout << name_ << " " << total_ << std::endl;
                    }
                }
            }
            
            void setPrint(bool print)
            {
                print_ = print;
            }
            
            void inc(int num = 1)
            {
                __sync_add_and_fetch(&total_, num);
            }
        
        private:
            SimpleCounter (const SimpleCounter&);
            SimpleCounter& operator= (const SimpleCounter&);
            std::string name_;
            long total_;
            bool print_;
    };
    
    
    namespace
    {
        SimpleCounter global_numGeometricCacheHits("[Katana Producer Cache Summary]: Number of Geometric Cache Hits:");
        SimpleCounter global_numGeometricQueueHits("[Katana Producer Cache Summary]: Number of Geometric Queue Hits:");
        SimpleCounter global_numInstanceCacheHits("[Katana Producer Cache Summary]: Number of Instance Cache Hits:");
        
        SimpleCounter global_numProducersInInstanceCache("[Katana Producer Cache Summary]: Instance Cache Size:");
        SimpleTimer global_totalRewalkTime("[Katana Producer Cache Summary]: Total Rewalk Time:");
        SimpleTimer global_instanceCacheWalkTime("[Katana Producer Cache Summary]: Instance Cache Sibling Traversal Time:");
        SimpleCounter global_instanceCacheWalkCounter("[Katana Producer Cache Summary]: Instance Cache Sibling Traversal Counter:");
        SimpleTimer global_downwardCacheWalkTime("[Katana Producer Cache Summary]: Downward Cache Rewalk Time:");
    }
}


///////////////////// PRODUCER UTILS /////////////////////////////////////////


namespace PRManProcedural
{
    std::string FindCommonScenegraphRoot(std::string locA, std::string locB)
    {
        // find the location of the 1st character that differs, and return
        // the nearest preceding '/'
        unsigned int slash_index = 0;
        std::string::size_type commonStringSize =
          std::min(locA.size(),locB.size());
        
        for(unsigned int index = 0; index < commonStringSize; ++index)
        {
            if(locA[index]!=locB[index])
            {
                // We've found differences.
                //Return up to the location of the previous slash
                //return std::string(locA.c_str(), slash_index);
                return locA.substr(0, slash_index);
            }
            
            if(locA[index] == '/')
                slash_index = index;
        }
        
        // The strings (which match in length) match exactly.
        
        // Inspect the string that has remaining characters.
        // If the remaining character is a slash, return the common
        // sized substring
        //
        // Example: locA = /root/world/a
        //          locB = /root/world/a/b/d
        // Return /root/world/a
        
        if(locA.size() > locB.size())
        {
            if(locA[locB.size()] == '/')
            {
                return locA.substr(0, commonStringSize);
            }
        }
        else
        {
            if(locB[locA.size()] == '/')
            {
                return locA.substr(0, commonStringSize);
            }
        }
        
        // Otherwise, return up to the location of the previous slash
        // Example: locA = /root/world/a
        //          locB = /root/world/apple
        // Return /root/world
        
        return locA.substr(0, slash_index);
    }
    
    
    FnScenegraphIterator DoIteratorWalk(FnScenegraphIterator sourceIterator, std::string targetLocation, PRManPluginState* sharedState)
    {
        if(!sourceIterator.isValid())
            return sourceIterator;
        
        // Is the producer what we are looing for?
        std::string sourceLocation = sourceIterator.getFullName();
        if(sourceLocation == targetLocation)
        {
            return sourceIterator;
        }
        
        std::string commonParentLocation = FindCommonScenegraphRoot(sourceLocation, targetLocation);
        
        FnScenegraphIterator commonParent = sourceIterator;
        while(commonParent.isValid() && commonParent.getFullName() != commonParentLocation)
        {
            commonParent = commonParent.getParent();
        }
        if(!commonParent.isValid())
        {
            std::ostringstream os;

            os << "Common parent could not be found.\n";
            os << "\tSource Location: " << sourceLocation
               << "\n\tTarget Location: " << targetLocation
               << "\n\tCommon Parent Location: " << commonParentLocation;
            Report_Fatal(os.str());
        }
        
        // Walk down from the parent to the specified scenegraphLocation.
        FnScenegraphIterator childIterator = commonParent;
        std::string childPath = SPI_pystring::replace(targetLocation, commonParent.getFullName(), "", 1);
        
        if(sharedState->proceduralSettings.printCacheStatistics_debug)
        {
            Report_Info("[Katana Producer Cache Summary]: Walking " + childPath);
        }
        
        childPath = SPI_pystring::strip(childPath,"/");
        std::vector<std::string> childNames;
        SPI_pystring::split(childPath, childNames,"/");
        
        for(unsigned int i=0; i<childNames.size(); i++)
        {
            if(!childIterator.isValid())
            {
                Report_Fatal("DoIteratorWalk lookup failed. Child could not be found.");
            }
            childIterator = childIterator.getChildByName(childNames[i]);
        }
        if(!childIterator.isValid())
        {
            Report_Fatal("DoIteratorWalk lookup failed. Child could not be found.");
        }
        
        return childIterator;
    }
}



namespace PRManProcedural
{
    ////////// GEOMETRIC  (FRUSTUM SCORED) CACHING       ////////////////////////////
    
    namespace
    {
        // TODO: Handle multi-sample bounds attr
        void _GetBoundsAttrValue(DoubleAttribute boundAttr,
                                 Imath::V3d* minBound, Imath::V3d* maxBound)
        {
            FnAttribute::DoubleConstVector bound = boundAttr.getNearestSample(0);

            if (!boundAttr.isValid() || bound.size() != 6)
            {
                throw std::runtime_error(std::string("Invalid bounds attr"));
            }
            
            (*minBound)[0] = bound[0];
            (*maxBound)[0] = bound[1];
            (*minBound)[1] = bound[2];
            (*maxBound)[1] = bound[3];
            (*minBound)[2] = bound[4];
            (*maxBound)[2] = bound[5];
        }
        
        void _GetTransformedBoundsAttrValue(DoubleAttribute boundAttr,
                                            const Imath::M44d& xform, std::vector<Imath::V3d>* outPoints)
        {
            std::vector<Imath::V3d>& points = *outPoints;
            Imath::V3d minBound; Imath::V3d maxBound;
            _GetBoundsAttrValue(boundAttr, &minBound, &maxBound);
            
            points.push_back(Imath::V3d(minBound[0], minBound[1], minBound[2]));
            points.push_back(Imath::V3d(minBound[0], minBound[1], maxBound[2]));
            points.push_back(Imath::V3d(minBound[0], maxBound[1], minBound[2]));
            points.push_back(Imath::V3d(minBound[0], maxBound[1], maxBound[2]));
            points.push_back(Imath::V3d(maxBound[0], minBound[1], minBound[2]));
            points.push_back(Imath::V3d(maxBound[0], minBound[1], maxBound[2]));
            points.push_back(Imath::V3d(maxBound[0], maxBound[1], minBound[2]));
            points.push_back(Imath::V3d(maxBound[0], maxBound[1], maxBound[2]));
            
            for (std::vector<Imath::V3d>::iterator it = points.begin(); it != points.end(); ++it)
            {
                xform.multVecMatrix(*it, *it);
            }
        }
        
        namespace
        {
            M44d computeXform(const FnAttribute::GroupAttribute & xformAttr, bool* isGlobal)
            {
                std::pair<FnAttribute::DoubleAttribute, bool> result = 
                    FnGeolibServices::FnXFormUtil::CalcTransformMatrixAtTime(
                        xformAttr, 0.f);
                *isGlobal = result.second;
                if (!result.first.isValid()) return M44d();
                FnAttribute::DoubleAttribute::array_type v = result.first.getNearestSample(0.f);
                if (v.size() < 16) return M44d();
                return M44d((double(*)[4]) v.data());
            }
        }
        
        // Values above 0 are in frustum
        // Values below 0 are outside
        float GetProducerCacheScore(FnScenegraphIterator sgIterator, FnAttribute::DoubleAttribute parentBoundsAttr, PRManPluginState* sharedState)
        {
            // DETERMINE BOUNDS LOCATION IN SCREEN SPACE
            
            FnScenegraphIterator parentIterator = sgIterator.getParent();
            if(!parentIterator.isValid())
                return false;
            
            FnAttribute::GroupAttribute xformgroup = sgIterator.getGlobalXFormGroup();

            bool isGlobal = false;
            M44d worldXform = computeXform(xformgroup, &isGlobal);

            std::vector<V3d> points;
            _GetTransformedBoundsAttrValue(parentBoundsAttr, worldXform, &points);
            
            // Create points representing the corner of the box
            std::auto_ptr<RtPoint> rtpoints(new RtPoint[points.size()]);
            for(unsigned int i=0; i<points.size(); ++i)
            {
                rtpoints.get()[i][0] = points[i][0];
                rtpoints.get()[i][1] = points[i][1];
                rtpoints.get()[i][2] = points[i][2];
            }

            float pMin[3];
            float pMax[3];

            // Assume a shutter-normalized time of 0.0 for the transform.
            RtInt result = RxTransformPoints((char*)"world", (char*)"NDC", points.size(), rtpoints.get(), 0.0);
            if(result)
            {
                if(sharedState->proceduralSettings.printCacheStatistics_debug)
                {
                    std::ostringstream  os;
                    os << "[Katana Producer Cache]: Error calculating cache score for "
                              << "\"" << sgIterator.getFullName() << "\": ";

                    if(result == -1)
                    {
                        os << "Coordinate system 'world' or 'NDC' not defined.";
                    }
                    else if(result == -2)
                    {
                        os << "Singular transform matrix provided.";
                    }
                    else
                    {
                        os << "Unexpected result returned from RxTransformPoints(): " << int(result);
                    }

                    Report_Error(os.str());
                }
                return false;
            }
            
            // Compute new bounding pts
            for(unsigned int i=0; i<points.size(); ++i)
            {
                if(i==0)
                {
                    for(int j=0;j<3;++j)
                    {
                        pMin[j] = rtpoints.get()[i][j];
                        pMax[j] = rtpoints.get()[i][j];
                    }
                }
                else
                {
                    for(int j=0;j<3;++j)
                    {
                        pMin[j] = std::min(rtpoints.get()[i][j],pMin[j]);
                        pMax[j] = std::max(rtpoints.get()[i][j],pMax[j]);
                    }
                }
            }
            
            // Find the manhattan distance to the nearest edge of crop window
            float distance = 0.0;
            if(pMax[0] < sharedState->proceduralSettings.cropWindow[0])
                distance += (sharedState->proceduralSettings.cropWindow[0] - pMax[0]);
            else if(pMin[0] > sharedState->proceduralSettings.cropWindow[1])
                distance += (pMin[0] - sharedState->proceduralSettings.cropWindow[1]);
            
            if(pMax[1] < sharedState->proceduralSettings.cropWindow[2])
                distance += (sharedState->proceduralSettings.cropWindow[2] - pMax[1]);
            else if(pMin[1] > sharedState->proceduralSettings.cropWindow[3])
                distance += (pMin[1] - sharedState->proceduralSettings.cropWindow[3]);
            
            // Assume 0,1 zdepth for crop window in NDC space
            if(pMax[2] < 0.0)
                distance += (0.0 - pMax[1]);
            else if(pMin[1] > 1.0)
                distance += (pMin[1] - 1.0);
            
            // If the distance to the edge is positive, return -distance as its score
            if(distance>0.0)
                return -distance;
            
            // Otherwise, for in frustum objects return 1.0 - ndc depth (closer objects
            // are scored higher)
            return 1.0 - std::max(0.0f,pMin[0]);

        }
        
        
        struct GeometricCachePacket
        {
            FnScenegraphIterator sgIterator;
            FnAttribute::DoubleAttribute parentBoundsAttr;
            float cacheScore;
            
            GeometricCachePacket():
                sgIterator(FnScenegraphIterator()), // init an invalid Sg Iterator
                cacheScore(0.0)
            {}
            
            GeometricCachePacket(FnScenegraphIterator sgIterator, FnAttribute::DoubleAttribute parentBoundsAttr):
                sgIterator(sgIterator),
                parentBoundsAttr(parentBoundsAttr),
                cacheScore(0.0)
            {}
        };
        
        
        // Protect access to all global cache structures via this mutex.
        boost::recursive_mutex global_cacheMutex;

        // The only producers that are or are not cached are those that 
        // will be subdivided for bounds calls. This is a much smaller
        // set than all scenegraph locations in the universe.
        std::map<std::string, GeometricCachePacket> global_geometricCache;
        std::list<GeometricCachePacket> global_geometricQueue;

        // The root iterator is required in the event of a cache miss so we can
        // walk the hierarchy to the requested location.
        FnScenegraphIterator global_rootIterator;

        std::map<std::string, InstanceCache> global_instanceCaches;
    }
    
    void RegisterRootSgIteratorInCache(FnKat::FnScenegraphIterator rootIterator)
    {
      boost::recursive_mutex::scoped_lock lock(global_cacheMutex);
      global_rootIterator = rootIterator;
    }

    void PushProducerIntoGeometricCache(FnScenegraphIterator sgIterator, FnAttribute::DoubleAttribute parentBoundsAttr, PRManPluginState* sharedState)
    {
        if(!sgIterator.isValid())
            return;
        if(!parentBoundsAttr.isValid())
            return;

        boost::recursive_mutex::scoped_lock lock(global_cacheMutex);

        // Add the new producer into the geometric queue
        {
            global_geometricQueue.push_back( GeometricCachePacket(sgIterator, parentBoundsAttr) );
        }
        
        // Queue Cleanup: If the queue is too large, empty into the cache-map
        while(global_geometricQueue.size() > sharedState->proceduralSettings.geometricCacheInputQueueLength)
        {
            // Pop a cache packet from the front of the queue
            GeometricCachePacket cachePacket = global_geometricQueue.front();
            global_geometricQueue.pop_front();
            
            // Compute its cache score
            cachePacket.cacheScore = GetProducerCacheScore(cachePacket.sgIterator, cachePacket.parentBoundsAttr, sharedState);
            
            if(sharedState->proceduralSettings.printCacheStatistics_debug)
            {
                std::ostringstream os;
                os << "[Katana Producer Cache Summary]: Adding "
                   << cachePacket.sgIterator.getFullName()
                   << " to geometric cache, score = "
                   << cachePacket.cacheScore;
                Report_Info(os.str());
            }
            
            // Add it into the cache
            global_geometricCache[cachePacket.sgIterator.getFullName()] = cachePacket;
        }
        
        // Cache-map Cleanup: if the cache is too large, drop the lowest scoring entry
        while(global_geometricCache.size() > sharedState->proceduralSettings.geometricProducerCacheLimit)
        {
            float lowestScore = 0;
            std::string lowestLocation;
            
            // Find the lowest score
            for(std::map<std::string, GeometricCachePacket>::iterator iter = global_geometricCache.begin();
                iter != global_geometricCache.end(); ++iter)
            {
                if(iter == global_geometricCache.begin())
                {
                    lowestScore = (*iter).second.cacheScore;
                    lowestLocation = (*iter).first;
                }
                else if((*iter).second.cacheScore < lowestScore)
                {
                    lowestScore = (*iter).second.cacheScore;
                    lowestLocation = (*iter).first;
                }
            }
            
            if(sharedState->proceduralSettings.printCacheStatistics_debug)
            {
                std::ostringstream os;
                os << "[Katana Producer Cache Summary]: Dropping "
                   << lowestLocation
                   << " from geometric cache, score = "
                   << lowestScore;
                Report_Info(os.str());
            }
            
            global_geometricCache.erase(lowestLocation);
        }
    }
    

    
    ////////// INSTANCE  CACHING       ////////////////////////////
    
    
    void RegisterInstanceCache(std::string location, InstanceCache instanceCache, int numUniqueProducers, PRManPluginState* sharedState)
    {
        if(instanceCache.size() < sharedState->proceduralSettings.producerInstanceCacheInterval)
        {
            return;
        }
        
        if(sharedState->proceduralSettings.printCacheStatistics_debug)
        {
            std::ostringstream os;
            os << "[Katana Producer Cache Summary]: Registering instance cache "
               << location
               << " numUniqueProducers "
               << numUniqueProducers
               << std::endl;
            Report_Info(os.str());
        }
        
        boost::recursive_mutex::scoped_lock lock(global_cacheMutex);
        global_numProducersInInstanceCache.inc(numUniqueProducers);
        global_instanceCaches[location] = instanceCache;
    }
    
    // Get the closest common parent from the instance cache
    std::string GetClosestCommonParentFromInstanceCache(std::string location)
    {
        boost::recursive_mutex::scoped_lock lock(global_cacheMutex);

        if(global_instanceCaches.size() == 0)
        {
            return "";
        }
        
        // Find the closest level cache
        std::string commonScenegraphRoot;
        for(std::map<std::string, InstanceCache>::iterator iter = global_instanceCaches.begin();
                iter != global_instanceCaches.end(); ++iter)
        {
            std::string localRoot = FindCommonScenegraphRoot(location, (*iter).first);
            if(localRoot.size() > commonScenegraphRoot.size())
            {
                commonScenegraphRoot = localRoot;
            }
        }
        
        // No closest level means no cache
        if(commonScenegraphRoot.empty())
        {
            return "";
        }
        
        // Assume the child's gonna exist, do some string manipulation to return the immediate child
        std::string childPath = SPI_pystring::replace(location, commonScenegraphRoot, "", 1);
        childPath = SPI_pystring::strip(childPath,"/");
        std::vector<std::string> childNames;
        SPI_pystring::split(childPath, childNames,"/",1);
        if(!childNames.size())
        {
            return "";
        }
        
        return commonScenegraphRoot + "/" + childNames[0];
    
    }
    
    FnScenegraphIterator GetProducerFromInstanceCache(std::string location)
    {
        boost::recursive_mutex::scoped_lock lock(global_cacheMutex);

        if(global_instanceCaches.size() == 0)
        {
            return FnScenegraphIterator();
        }
        
        // Find the closest level cache
        std::string commonScenegraphRoot;
        for(std::map<std::string, InstanceCache>::iterator iter = global_instanceCaches.begin();
                iter != global_instanceCaches.end(); ++iter)
        {
            std::string localRoot = FindCommonScenegraphRoot(location, (*iter).first);
            if(localRoot.size() > commonScenegraphRoot.size())
            {
                commonScenegraphRoot = localRoot;
            }
        }
        
        if(commonScenegraphRoot.empty())
        {
            return FnScenegraphIterator();
        }
        
        
        std::map<std::string, InstanceCache>::iterator localInstanceCacheIter = global_instanceCaches.find(commonScenegraphRoot);
        if(localInstanceCacheIter == global_instanceCaches.end())
        {
            // This cannot happen, as gotten the key by finding the map!
            return FnScenegraphIterator();
        }
        
        std::string childPath = SPI_pystring::replace(location, commonScenegraphRoot, "", 1);
        childPath = SPI_pystring::strip(childPath,"/");
        std::vector<std::string> childNames;
        SPI_pystring::split(childPath, childNames,"/",1);
        if(!childNames.size())
        {
            // This should not happen, a child was requested that was never created!
            // And all children are alwasy created synchronously as part of the expansion,
            // so how'd we get here?
            
            // TODO: We may be requesting the parent of a cached location. Perhaps find the 1st entry and return 1 up?
            return FnScenegraphIterator();
        }
        
        
        FnScenegraphIterator sgIterator;
        
        InstanceCache::iterator iter = (*localInstanceCacheIter).second.find(childNames[0]);
        if(iter == (*localInstanceCacheIter).second.end())
        {
            std::ostringstream os;
            os << "Could not find an extry for child '"
               << childNames[0]
               << "' at InstanceCache "
               <<  commonScenegraphRoot
               << "\n";
            os << "\tClosest Producer From Instance Cache " << location;
            os << "\n\tCommon Scenegraph Root " << commonScenegraphRoot;
            os << "\n\tChild Path " << childPath;
            Report_Error(os.str());

            return FnScenegraphIterator();
        }
        
        sgIterator = (*iter).second;
        if(!sgIterator.isValid())
        {
            std::ostringstream os;
            os << "Invalid Iterator found in InstanceCache \n";
            os << "\tCommon Scenegraph Root " << commonScenegraphRoot;
            os << "\n\tChild Path " << childPath << std::endl;
            Report_Error(os.str());
            return FnScenegraphIterator();
        }
        
        
        global_instanceCacheWalkTime.start();
        
        while(sgIterator.isValid() && sgIterator.getName()!=childNames[0])
        {
            sgIterator = sgIterator.getNextSibling();
            global_instanceCacheWalkCounter.inc();
        }
        
        global_instanceCacheWalkTime.stop();
        
        return sgIterator;
    }

    FnScenegraphIterator GetCachedSgIterator(std::string scenegraphLocation, PRManPluginState* sharedState)
    {
        if(scenegraphLocation.empty())
        {
            Report_Fatal("GetCachedSgIterator asked for null scenegraphLocation.");
        }
        
        // This technically only needs to be done once at startup, but it's so cheap compared to the caching that who cares.
        {
            global_totalRewalkTime.setPrint(sharedState->proceduralSettings.printCacheStatistics_summary);
            global_instanceCacheWalkTime.setPrint(sharedState->proceduralSettings.printCacheStatistics_summary);
            global_instanceCacheWalkCounter.setPrint(sharedState->proceduralSettings.printCacheStatistics_summary);
            global_downwardCacheWalkTime.setPrint(sharedState->proceduralSettings.printCacheStatistics_summary);
            global_numGeometricCacheHits.setPrint(sharedState->proceduralSettings.printCacheStatistics_summary);
            global_numGeometricQueueHits.setPrint(sharedState->proceduralSettings.printCacheStatistics_summary);
            global_numInstanceCacheHits.setPrint(sharedState->proceduralSettings.printCacheStatistics_summary);
            global_numProducersInInstanceCache.setPrint(sharedState->proceduralSettings.printCacheStatistics_summary);
        }
        
        boost::recursive_mutex::scoped_lock lock(global_cacheMutex);

        // Is the requested scenegraph location in the geometric producer queue?
        for(std::list<GeometricCachePacket>::iterator iter = global_geometricQueue.begin();
                iter != global_geometricQueue.end(); ++iter)
        {
            if((*iter).sgIterator.getFullName() == scenegraphLocation)
            {
                FnScenegraphIterator sgIterator = (*iter).sgIterator;
                global_geometricQueue.erase(iter);
                
                global_numGeometricQueueHits.inc();
                if(sharedState->proceduralSettings.printCacheStatistics_debug)
                {
                    Report_Info("[Katana SgIterator Cache Summary]: Geometric Queue Hit " + scenegraphLocation);
                }
                
                return sgIterator;
            }
        }
        
        // No?  How about in the geometric producer cache?
        std::map<std::string, GeometricCachePacket>::iterator result  = global_geometricCache.find(scenegraphLocation);
        if(result != global_geometricCache.end())
        {
            global_numGeometricCacheHits.inc();
            if(sharedState->proceduralSettings.printCacheStatistics_debug)
            {
                Report_Info("[Katana Producer Cache Summary]: Cache hit " + scenegraphLocation);
            }
            
            FnScenegraphIterator sgIterator = (*result).second.sgIterator;
            global_geometricCache.erase(scenegraphLocation);
            return sgIterator;
        }
        
        // No? So it's a cache miss.  Search everything we've got to find the closest parent.
        global_totalRewalkTime.start();
        
        if(!global_rootIterator.isValid())
        {
            if(sharedState->proceduralSettings.printCacheStatistics_debug)
            {
                Report_Error("[Katana Producer Cache Summary]: "
                             "Cache miss lookup failed as root iterator is "
                             "not valid or has not been set.");
            }
            return FnScenegraphIterator();
        }

        FnScenegraphIterator startingSgIterator = global_rootIterator;
        std::string startingSgIteratorScenegraphLocation = startingSgIterator.getFullName();
        
        // Check all locations in the geometric queue
        for(std::list<GeometricCachePacket>::iterator iter = global_geometricQueue.begin();
                iter != global_geometricQueue.end(); ++iter)
        {
            std::string commonRoot = FindCommonScenegraphRoot(scenegraphLocation, (*iter).sgIterator.getFullName());
            if(commonRoot.size() > startingSgIteratorScenegraphLocation.size())
            {
                startingSgIteratorScenegraphLocation = commonRoot;
                startingSgIterator = (*iter).sgIterator;
            }
        }
        
        // Check all locations in the geometric cache
        for(std::map<std::string, GeometricCachePacket>::iterator iter = global_geometricCache.begin();
            iter != global_geometricCache.end(); ++iter)
        {
            std::string commonRoot = FindCommonScenegraphRoot(scenegraphLocation, (*iter).first);
            if(commonRoot.size() > startingSgIteratorScenegraphLocation.size())
            {
                startingSgIteratorScenegraphLocation = commonRoot;
                startingSgIterator = (*iter).second.sgIterator;
            }
        }
        
        // Check the instance cache for a closer producer
        std::string closestInstanceLocation = GetClosestCommonParentFromInstanceCache(scenegraphLocation);
        if(closestInstanceLocation.size() > startingSgIteratorScenegraphLocation.size())
        {
            FnScenegraphIterator closerSgIterator = GetProducerFromInstanceCache(closestInstanceLocation);
            if(closerSgIterator.isValid())
            {
                if(sharedState->proceduralSettings.printCacheStatistics_debug)
                {
                    Report_Info("[Katana Producer Cache Summary]: Instance Cache Hit " + closerSgIterator.getFullName());
                }
                
                global_numInstanceCacheHits.inc();
                
                startingSgIterator = closerSgIterator;
            }
        }
        
        // Walk from the closest producer to the required location.
        global_downwardCacheWalkTime.start();
        FnScenegraphIterator targetSgIterator = DoIteratorWalk(startingSgIterator, scenegraphLocation, sharedState);
        global_downwardCacheWalkTime.stop();
        
        global_totalRewalkTime.stop();
        return targetSgIterator;
    }
}

