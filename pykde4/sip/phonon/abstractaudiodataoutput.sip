// Copyright 2008 Simon Edwards <simon@simonzone.com>

//                 Generated by twine

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as
// published by the Free Software Foundation; either version 2, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details

// You should have received a copy of the GNU Library General Public
// License along with this program; if not, write to the
// Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

namespace Phonon
{
namespace Experimental
{

class AbstractAudioDataOutput : Phonon::MediaNode /NoDefaultCtors,Abstract/
{
%TypeHeaderCode
#include <phonon/experimental/abstractaudiodataoutput.h>
%End

public:
                            AbstractAudioDataOutput ();
    virtual QSet<Phonon::Experimental::AudioFormat>  allowedFormats () const;
    void                    setAllowedFormats (const QSet<Phonon::Experimental::AudioFormat>&);
    bool                    isRunning () const;
//FIXME    virtual void            packetReady (const Phonon::Packet&) = 0;
    void                    endOfMedia ();
    void                    setRunning (bool running);
    void                    start ();
    void                    stop ();

protected:


//mt undefined symbol: AbstractAudioDataOutputPrivate& dd -- need mapped type?
//mt                        AbstractAudioDataOutput (AbstractAudioDataOutputPrivate& dd);


};   // AbstractAudioDataOutput

};   // Experimental

};   // Phonon
/*
%MappedType QSet<Phonon::Experimental::AudioFormat>
{
%TypeHeaderCode
#include <qset.h>
#include <phonon/experimental/abstractaudiodataoutput.h>
%End

%ConvertFromTypeCode
    // Create the list.
    PyObject *l;

    if ((l = PyList_New(sipCpp->size())) == NULL)
        return NULL;

    // Set the list elements.
    QSet<Phonon::Experimental::AudioFormat> set = *sipCpp;
    int i = 0;
    foreach (Phonon::Experimental::AudioFormat value, set)
    {
        PyObject *obj = PyInt_FromLong ((long) value);
        if (obj == NULL || PyList_SET_ITEM (l, i, obj) < 0)
        {
            Py_DECREF(l);

            if (obj)
                Py_DECREF(obj);

            return NULL;
        }

        Py_DECREF(obj);
        i++;
    }
   
    return l;
%End

%ConvertToTypeCode
    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyList_Check(sipPy))
            return 0;
    }

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyList_Check(sipPy))
            return 0;
    }

    QSet<Phonon::Experimental::AudioFormat> *qs = new QSet<Phonon::Experimental::AudioFormat>;
 
    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
    {
        Phonon::Experimental::AudioFormat t = (Phonon::Experimental::AudioFormat)PyInt_AS_LONG (PyList_GET_ITEM (sipPy, i));
        *qs << t;

    }
 
    *sipCppPtr = qs;
 
    return sipGetState(sipTransferObj);
%End
};
*/