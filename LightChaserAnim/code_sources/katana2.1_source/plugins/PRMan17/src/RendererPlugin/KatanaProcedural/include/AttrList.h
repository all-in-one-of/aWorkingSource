// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef ATTRLIST_H
#define ATTRLIST_H

#include <list>
#include <vector>
#include <map>
#include <set>
#include <ri.h>

namespace PRManProcedural
{
    // The various WriteRI_XXXXX routines build up AttrLists to pass to PRMan.
    // These are high-level constructs, defined in terms of attributes and names.
    // Helper routines convert these into token and param lists for passing to PRMan
    // calls.  If this seems indirect, consider that it's so darn convenient to
    // build up these AttrLists out of objects that correctly manage the underlying
    // memory... and then have a little chunk of code before the actual RiFoo call
    // convert to the C-style RenderMan format.

    struct AttrListEntry
    {
        AttrListEntry(const std::string& declaration,
                      const std::string& name,
                      FnAttribute::Attribute value,
                      bool mightBlur = false) : _declaration(declaration),
                                                _name(name),
                                                _value(value),
                                                _mightBlur(mightBlur)
        {
        }

        // e.g. "uniform string ColMap"
        std::string _declaration;

        // e.g. "ColMap"
        std::string _name;

        // The value(s) for this attribute.
        FnAttribute::Attribute _value;

        // Does motion blur apply?
        bool _mightBlur;
    };
    typedef std::list<AttrListEntry> AttrList;

    class AttrList_Converter
    {
    // Scoping object to convert an AttrList to the structures expected by prman.
    // This class is defined in WriteRI_Util.cpp
    public:
        AttrList_Converter(const AttrList& l);
        AttrList_Converter(const AttrList& l, const std::set<float>& samples);

        unsigned long getNumSampleTimes() const;
        std::vector<float>& getSampleTimes()
        {
            return _sampleTimes;
        }
        unsigned long getSize(float t);
        RtToken* getNameTokens(float t);
        RtToken* getDeclarationTokens(float t);
        RtPointer* getParameters(float);

    private:
        std::vector<float> _sampleTimes;
        std::map<float, std::vector<RtToken> > _nameTokenMap;
        std::map<float, std::vector<RtToken> > _declTokenMap;
        std::map<float, std::vector<RtPointer> > _paramMap;
        std::list<std::vector<RtFloat> > _floatVectors;

        RtPointer AttrToParam(FnAttribute::Attribute attr, const std::string& decl, float time);
        void init(const AttrList& attrList, const std::set<float>& samples);
    };

    void AddEntry(AttrList* attrList, AttrListEntry entry);
}

#endif
