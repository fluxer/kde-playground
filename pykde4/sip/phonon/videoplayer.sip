//
//     Copyright 2008 Simon Edwards <simon@simonzone.com>

//                 Generated by twine

// This file is part of PyKDE4.

// PyKDE4 is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of
// the License, or (at your option) any later version.

// PyKDE4 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

namespace Phonon
{

class VideoPlayer : QWidget
{
%TypeHeaderCode
#include <phonon/videoplayer.h>
%End

public:
    explicit                VideoPlayer (Phonon::Category category, QWidget* parent /TransferThis/ = 0);
                            VideoPlayer (QWidget* parent /TransferThis/ = 0);
    qint64                  totalTime () const;
    qint64                  currentTime () const;
    float                   volume () const;
    bool                    isPlaying () const;
    bool                    isPaused () const;
    Phonon::MediaObject*    mediaObject () const;
    Phonon::AudioOutput*    audioOutput () const;
    Phonon::VideoWidget*    videoWidget () const;
    void                    load (const Phonon::MediaSource& source);
    void                    play (const Phonon::MediaSource& source);
    void                    play ();
    void                    pause ();
    void                    stop ();
    void                    seek (qint64 ms);
    void                    setVolume (float volume);

signals:
    void                    finished ();
public:
    ~VideoPlayer ();
protected:
    bool                    event (QEvent*);
};
// VideoPlayer

};
// Phonon


