ID3 tag version 2.3.0
=====================




This document is an informal standard and replaces the ID3v2.2.0 standard [ID3v2](#1). The informal standard is released so that implementors could have a set standard before a formal standard is set. The formal standard will use another version or revision number if not identical to what is described in this document. The contents in this document may change for clarifications but never for added or altered functionallity.

Distribution of this document is unlimited.

1. Abstract
-----------

This document describes the ID3v2.3.0, which is a more developed version of the ID3v2 informal standard ID3v2 (version 2.2.0), evolved from the ID3 tagging system. The ID3v2 offers a flexible way of storing information about an audio file within itself to determine its origin and contents. The information may be technical information, such as equalisation curves, as well as related meta information, such as title, performer, copyright etc.



2. Conventions in this document
-------------------------------

In the examples, text within "" is a text string exactly as it appears in a file. Numbers preceded with $ are hexadecimal and numbers preceded with % are binary. $xx is used to indicate a byte with unknown content. %x is used to indicate a bit with unknown content. The most significant bit (MSB) of a byte is called 'bit 7' and the least significant bit (LSB) is called 'bit 0'.

A tag is the whole tag described in this document. A frame is a block of information in the tag. The tag consists of a header, frames and optional padding. A field is a piece of information; one value, a string etc. A numeric string is a string that consists of the characters 0-9 only.



3. ID3v2 overview
-----------------

The two biggest design goals were to be able to implement ID3v2 without disturbing old software too much and that ID3v2 should be as flexible and expandable as possible.

The first criterion is met by the simple fact that the MPEG [MPEG] decoding software uses a syncsignal, embedded in the audiostream, to 'lock on to' the audio. Since the ID3v2 tag doesn't contain a valid syncsignal, no software will attempt to play the tag. If, for any reason, coincidence make a syncsignal appear within the tag it will be taken care of by the 'unsynchronisation scheme' described in _section [5](#5-the-unsynchronization-scheme)_.

The second criterion has made a more noticeable impact on the design of the ID3v2 tag. It is constructed as a container for several information blocks, called frames, whose format need not be known to the software that encounters them. At the start of every frame there is an identifier that explains the frames' format and content, and a size descriptor that allows software to skip unknown frames.

If a total revision of the ID3v2 tag should be needed, there is a version number and a size descriptor in the ID3v2 header.

The ID3 tag described in this document is mainly targeted at files encoded with MPEG-1/2 layer I, MPEG-1/2 layer II, MPEG-1/2 layer III and MPEG-2.5, but may work with other types of encoded audio.

The bitorder in ID3v2 is most significant bit first (MSB). The byteorder in multibyte numbers is most significant byte first (e.g. $12345678 would be encoded $12 34 56 78).

It is permitted to include padding after all the final frame (at the end of the ID3 tag), making the size of all the frames together smaller than the size given in the head of the tag. A possible purpose of this padding is to allow for adding a few additional frames or enlarge existing frames within the tag without having to rewrite the entire file. The value of the padding bytes must be $00.


### 3.1. ID3v2 header

The ID3v2 tag header, which should be the first information in the file, is 10 bytes as follows:

    id3v2_file_identifier       "ID3"
    id3v2_version               $03 00
    id3v2_flags                 %abc00000
    id3v2_size                  4 * %0xxxxxxx

The first three bytes of the tag are always `"ID3"` to indicate that this is an ID3v2 tag, directly followed by the two version bytes. The first byte of ID3v2 version is it's major version, while the second byte is its revision number. In this case this is ID3v2.3.0. All revisions are backwards compatible while major versions are not. If software with ID3v2.2.0 and below support should encounter version three or higher it should simply ignore the whole tag. Version and revision will never be `$FF`.

The version is followed by one the ID3v2 flags field, of which currently only three flags are used.

- **`a`** unsynchronisation  
    Bit 7 in the 'ID3v2 flags' indicates whether or not unsynchronisation is used (see section 5 for details); a set bit indicates usage.

- **`b`** extended header  
    The second bit (bit 6) indicates whether or not the header is followed by an extended header. The extended header is described in section 3.2.

- **`c`** experimental indicator  
The third bit (bit 5) should be used as an 'experimental indicator'. This flag should always be set when the tag is in an experimental stage.

All the other flags should be cleared. If one of these undefined flags are set that might mean that the tag is not readable for a parser that does not know the flags function.

The ID3v2 tag size is encoded with four bytes where the most significant bit (bit 7) is set to zero in every byte, making a total of 28 bits. The zeroed bits are ignored, so a 257 bytes long tag is represented as `$00 00 02 01`.

The ID3v2 tag size is the size of the complete tag after unsychronisation, including padding, excluding the header but not excluding the extended header (`total_tag_size - 10` bytes). Only 28 bits (representing up to 256MB) are used in the size description to avoid the introduction of _false syncsignals_.

An ID3v2 tag can be detected with the following pattern:

    $49 44 33 yy yy xx zz zz zz zz
    
Where `yy` is less than `$FF`, `xx` is the _flags_ byte and `zz` is less than `$80`.


### 3.2. ID3v2 extended header

The extended header contains information that is not vital to the correct parsing of the tag information, hence the extended header is optional.

    extended_header_size        $xx xx xx xx
    extended_flags              $xx xx
    size_of_padding             $xx xx xx xx

Where the 'Extended header size', currently 6 or 10 bytes, excludes itself. The 'Size of padding' is simply the total tag size excluding the frames and the headers, in other words the padding. The extended header is considered separate from the header proper, and as such is subject to unsynchronisation.

The extended flags are a secondary flag set which describes further attributes of the tag. These attributes are currently defined as follows

     %x0000000 00000000

- **`x`** CRC data present  
If this flag is set four bytes of CRC-32 data is appended to the extended header. The CRC should be calculated before unsynchronisation on the data between the extended header and the padding, i.e. the frames and only the frames.

    total_frame_CRC             $xx xx xx xx


### 3.3. ID3v2 frame overview

As the tag consists of a tag header and a tag body with one or more frames, all the frames consists of a frame header followed by one or more fields containing the actual information. The layout of the frame header:

     frame_id                   $xx xx xx xx  (four characters)
     frame_size                 $xx xx xx xx
     frame_flags                $xx xx

The frame ID made out of the characters capital `A-Z` and `0-9`. Identifiers beginning with `"X"`, `"Y"` and `"Z"` are for experimental use and free for everyone to use, without the need to set the experimental bit in the tag header. Have in mind that someone else might have used the same identifier as you. All other identifiers are either used or reserved for future use.

The frame ID is followed by a size descriptor, making a total header size of ten bytes in every frame. The size is calculated as frame size excluding frame header (`frame_size - 10`).

In the frame header the size descriptor is followed by two flags bytes. These flags are described in _section [3.3.1](#331-frame-header-flags)_.

There is no fixed order of the frames' appearance in the tag, although it is desired that the frames are arranged in order of significance concerning the recognition of the file. An example of such order: [`UFID`](#UFID), [`TIT2`](#TIT2), [`MCDI`](#MCDI), [`TRCK`](#TRCK) ...

A tag must contain at least one frame. A frame must be at least 1 byte big, excluding the header.

If nothing else is said a string is represented as ISO-8859-1 [ISO-8859-1] characters in the range `$20 - $FF`. Such strings are represented as `<text_string>`, or `<full_text_string>` if newlines are allowed, in the frame descriptions. All Unicode strings [UNICODE] use 16-bit unicode 2.0 (ISO/IEC 10646-1:1993, UCS-2). Unicode strings must begin with the Unicode BOM (`$FF FE or $FE FF`) to identify the byte order.

All numeric strings and URLs [URL] are always encoded as ISO-8859-1. Terminated strings are terminated with `$00` if encoded with ISO-8859-1 and `$00 00` if encoded as unicode. If nothing else is said newline character is forbidden. In ISO-8859-1 a new line is represented, when allowed, with `$0A` only. Frames that allow different types of text encoding have a text encoding description byte directly after the frame size. If ISO-8859-1 is used this byte should be `$00`, if Unicode is used it should be $01. Strings dependent on encoding is represented as `<text_string_according_to_encoding>`, or `<full_text_string_according_to_encoding>` if newlines are allowed. Any empty Unicode strings which are NULL-terminated may have the Unicode `BOM` followed by a Unicode `NULL` (`$FF FE 00 00 or $FE FF 00 00`).

The three byte language field is used to describe the language of the frame's content, according to ISO-639-2 [ISO-639-2].

All URLs [URL] may be relative, e.g. `"picture.png"`, `"../doc.txt"`.

If a frame is longer than it should be, e.g. having more fields than specified in this document, that indicates that additions to the frame have been made in a later version of the ID3v2 standard. This is reflected by the revision number in the header of the tag.


### 3.3.1. Frame header flags

In the frame header the size descriptor is followed by two flags bytes. All unused flags must be cleared. The first byte is for _status messages_ and the second byte is for _encoding purposes_. If an unknown flag is set in the first byte the frame may not be changed without the bit cleared. If an unknown flag is set in the second byte it is likely to not be readable. The flags field is defined as follows.

    %abc00000 %ijk00000

- **`a`** tag alter preservation  
    This flag tells the software what to do with this frame if it is unknown and the tag is altered in any way. This applies to all kinds of alterations, including adding more padding and reordering the frames.

    - `0`, frame should be preserved,
    - `1`, frame should be discarded.

- **`b`** file alter preservation  
    This flag tells the software what to do with this frame if it is unknown and the file, excluding the tag, is altered. This does not apply when the audio is completely replaced with other audio data.

    - `0`, frame should be preserved,
    - `1`, frame should be discarded.

- **`c`** read only  
    This flag, if set, tells the software that the contents of this frame is intended to be read only. Changing the contents might break something, e.g. a signature. If the contents are changed, without knowledge in why the frame was flagged read only and without taking the proper means to compensate, e.g. recalculating the signature, the bit should be cleared.

- **`i`** compression  
    This flag indicates whether or not the frame is compressed.

    - `0`, frame is not compressed,
    - `1`, frame is compressed using zlib [zlib] with 4 bytes for _decompressed size_ appended to the frame header.

- **`j`** encryption  
    This flag indicates whether or not the frame is encrypted. If set one byte indicating with which method it was encrypted will be appended to the frame header. See _section [4.26](#426-encryption-method-registration)_ for more information about encryption method registration.

    - `0`, frame is not encrypted,
    - `1`, frame is encrypted.

- **`k`** grouping identity  
    This flag indicates whether or not this frame belongs in a group with other frames. If set a group identifier byte is added to the frame header. Every frame with the same group identifier belongs to the same group.

    - `0`, frame does not contain group information,
    - `1`, frame contains group information.

Some flags indicates that the frame header is extended with additional information. This information will be added to the frame header in the same order as the flags indicating the additions. I.e. the four bytes of decompressed size will precede the encryption method byte. These additions to the frame header, while not included in the frame header size but are included in the `frame_size` field, are not subject to encryption or compression.


### 3.3.2. Default flags

The default settings for the frames described in this document can be divided into the following classes. The flags may be set differently if found more suitable by the software.

1. Discarded if tag is altered, discarded if file is altered,  
     None.
2. Discarded if tag is altered, preserved if file is altered,  
     None.
3. Preserved if tag is altered, discarded if file is altered,  
     [`AENC`](#AENC),
     [`ETCO`](#ETCO),
     [`EQUA`](#EQUA),
     [`MLLT`](#MLLT),
     [`POSS`](#POSS),
     [`SYLT`](#SYLT),
     [`SYTC`](#SYTC),
     [`RVAD`](#RVAD),
     [`TENC`](#TENC),
     [`TLEN`](#TLEN),
     [`TSIZ`](#TSIS)
4. Preserved if tag is altered, preserved if file is altered  
     The rest of the frames.



4. Declared ID3v2 frames
------------------------

The following frames are declared in this draft (in alphabetical order):

- [`AENC`](#420-audio-encryption) Audio encryption
- [`APIC`](#415-attached-picture) Attached picture
- [`COMM`](#411-comments) Comments
- [`COMR`](#425-commercial-frame) Commercial frame
- [`ENCR`](#426-encryption-method-registration) Encryption method registration
- [`EQUA`](#413-equalization) Equalization
- [`ETCO`](#46-event-timing-codes) Event timing codes
- [`GEOB`](#416-general-encapsulated-object) General encapsulated object
- [`GRID`](#427-group-identification-registration) Group identification registration
- [`IPLS`](#44-involved-people-list) Involved people list
- [`LINK`](#421-linked-information) Linked information
- [`MCDI`](#45-music-cd-identifier) Music CD identifier
- [`MLLT`](#47-mpeg-location-lookup-table) MPEG location lookup table
- [`OWNE`](#424-ownership-frame) Ownership frame
- [`PRIV`](#428-private-frame) Private frame
- [`PCNT`](#417-play-counter) Play counter
- [`POPM`](#418-popularimeter) Popularimeter
- [`POSS`](#422-position-synchronization-frame) Position synchronization frame
- [`RBUF`](#419-recommended-buffer-size) Recommended buffer size
- [`RVAD`](#412-relative-volume-adjustment) Relative volume adjustment
- [`RVRB`](#414-reverb) Reverb
- [`SYLT`](#410-synchronized-lyricstext) Synchronized lyrics/text
- [`SYTC`](#48-synchronized-tempo-codes) Synchronized tempo codes
- [`TALB`](#talb) Album/Movie/Show title
- [`TBPM`](#tbpm) BPM (beats per minute)
- [`TCOM`](#tcom) Composer
- [`TCON`](#tcon) Content type
- [`TCOP`](#tcop) Copyright message
- [`TDAT`](#tdat) Date
- [`TDLY`](#tdly) Playlist delay
- [`TENC`](#tenc) Encoded by
- [`TEXT`](#text) Lyricist/Text writer
- [`TFLT`](#tflt) File type
- [`TIME`](#time) Time
- [`TIT1`](#tit1) Content group description
- [`TIT2`](#tit2) Title/songname/content description
- [`TIT3`](#tit3) Subtitle/Description refinement
- [`TKEY`](#tkey) Initial key
- [`TLAN`](#tlan) Language(s)
- [`TLEN`](#tlen) Length
- [`TMED`](#tmed) Media type
- [`TOAL`](#toal) Original album/movie/show title
- [`TOFN`](#tofn) Original filename
- [`TOLY`](#toly) Original lyricist(s)/text writer(s)
- [`TOPE`](#tope) Original artist(s)/performer(s)
- [`TORY`](#tory) Original release year
- [`TOWN`](#town) File owner/licensee
- [`TPE1`](#tpe1) Lead performer(s)/Soloist(s)
- [`TPE2`](#tpe2) Band/orchestra/accompaniment
- [`TPE3`](#tpe3) Conductor/performer refinement
- [`TPE4`](#tpe4) Interpreted, remixed, or otherwise modified by
- [`TPOS`](#tpos) Part of a set
- [`TPUB`](#tpub) Publisher
- [`TRCK`](#trck) Track number/Position in set
- [`TRDA`](#trda) Recording dates
- [`TRSN`](#trsn) Internet radio station name
- [`TRSO`](#trso) Internet radio station owner
- [`TSIZ`](#tsiz) Size
- [`TSRC`](#tsrc) ISRC (international standard recording code)
- [`TSSE`](#tsse) Software/Hardware and settings used for encoding
- [`TYER`](#tyer) Year
- [`TXXX`](#422-user-defined-text-information-frame) User defined text information frame
- [`UFID`](#41-unique-file-identifier) Unique file identifier
- [`USER`](#423-terms-of-use) Terms of use
- [`USLT`](#49-unsynchronized-lyrics-text-transcription) Unsynchronized lyric/text transcription
- [`WCOM`](#431-commercial-information) Commercial information
- [`WCOP`](#431-copyright-legal-information) Copyright/Legal information
- [`WOAF`](#431-official-audio-file-web-page) Official audio file web page
- [`WOAR`](#431-official-artist-performer-web-page) Official artist/performer web page
- [`WOAS`](#431-official-audio-source-web-page) Official audio source web page
- [`WORS`](#431-official-internet-radio-station-homepage) 
- [`WPAY`](#431-payment) Payment
- [`WPUB`](#431-publishers-official-web-page) Publishers official web page
- [`WXXX`](#432-user-defined-url-link-frame) User defined URL link frame


### 4.1. Unique file identifier

This frame's purpose is to be able to identify the audio file in a database that may contain more information relevant to the content. Since standardization of such a database is beyond this document, all frames begin with a null-terminated string with a URL [URL] containing an email address, or a link to a location where an email address can be found, that belongs to the organization responsible for this specific database implementation. Questions regarding the database should be sent to the indicated email address. The URL should not be used for the actual database queries. The string "http://www.id3.org/dummy/ufid.html" should be used for tests. Software that isn't told otherwise may safely remove such frames. The _Owner identifier_ must be non-empty (more than just a termination). The _Owner identifier_ is then followed by the actual identifier, which may be up to 64 bytes. There may be more than one `UFID` frame in a tag, but only one with the same 'Owner identifier'.

    <Header for 'Unique file identifier', ID: "UFID">
    owner_identifier            <text_string> $00
    identifier                  <up to 64 bytes binary data>


### 4.2. Text information frames

The text information frames are the most important frames, containing information like artist, album and more. There may only be one text information frame of its kind in an tag. If the text string is followed by a termination (`$00 (00)`) all the following information should be ignored and not be displayed. All text frame identifiers begin with `"T"`. Only text frame identifiers begin with `"T"`, with the exception of the [`TXXX`](#422-user-defined-text-information-frame) frame. All the text information frames have the following format:

    <Header for 'Text information frame', ID: "T000" - "TZZZ", excluding "TXXX" described in 4.2.2.>
    text_encoding               $xx
    information                 <text_string_according_to_encoding>


#### 4.2.1. Text information frames - details

##### TALB
The _Album/Movie/Show title_ frame is intended for the title of the recording(/source of sound) which the audio in the file is taken from.

##### TBPM
The _BPM_ frame contains the number of beats per minute in the main part of the audio. The BPM is an integer and represented as a numerical string.

##### TCOM
The _Composer(s)_ frame is intended for the name of the composer(s). They are separated with the `"/"` character.

##### TCON
The _Content type_, which previously was stored as a one byte numeric value only, is now a numeric string. You may use one or several of the types as ID3v1.1 did or, since the category list would be impossible to maintain with accurate and up to date categories, define your own.

References to the ID3v1 genres can be made by, as first byte, enter `"("` followed by a number from the genres list (see _[appendix A](#appendix-a)_) and ended with a `")"` character. This is optionally followed by a refinement, e.g. `"(21)"` or `"(4)Eurodisco"`. Several references can be made in the same frame, e.g. `"(51)(39)"`. If the refinement should begin with a `"("` character it should be replaced with `"(("`, e.g. `"((I can figure out any genre)"` or `"(55)((I think...)"`. The following new content types is defined in ID3v2 and is implemented in the same way as the numeric content types, e.g. `"(RX)"`.

- `RX` = remix
- `CR` = cover

##### TCOP
The _Copyright message_ frame, which must begin with a year and a space character (making five characters), is intended for the copyright holder of the original sound, not the audio file itself. The absence of this frame means only that the copyright information is unavailable or has been removed, and must not be interpreted to mean that the sound is public domain. Every time this field is displayed the field must be preceded with `"Copyright © "`.

##### TDAT
The _Date_ frame is a numeric string in the `DDMM` format containing the date for the recording. This field is always four characters long.

##### TDLY
The _Playlist delay_ defines the numbers of milliseconds of silence between every song in a playlist. The player should use the "ETC" frame, if present, to skip initial silence and silence at the end of the audio to match the _Playlist delay_ time. The time is represented as a numeric string.

##### TENC
The _Encoded by_ frame contains the name of the person or organization that encoded the audio file. This field may contain a copyright message, if the audio file also is copyrighted by the encoder.

##### TEXT
The _Lyricist(s)/Text writer(s)_ frame is intended for the writer(s) of the text or lyrics in the recording. They are seperated with the `"/"` character.

##### TFLT
The _File type_ frame indicates which type of audio this tag defines. The following type and refinements are defined:

    MPG    MPEG Audio
      /1     MPEG 1/2 layer I
      /2     MPEG 1/2 layer II
      /3     MPEG 1/2 layer III
      /2.5   MPEG 2.5
      /AAC   Advanced audio compression
    VQF    Transform-domain Weighted Interleave Vector Quantization
    PCM    Pulse Code Modulated audio

but other types may be used, not for these types though. This is used in a similar way to the predefined types in the [`TMED`](#tmed) frame, but without parentheses. If this frame is not present audio type is assumed to be `"MPG"`.

##### TIME
The _Time_ frame is a numeric string in the HHMM format containing the time for the recording. This field is always four characters long.

##### TIT1
The _Content group description_ frame is used if the sound belongs to a larger category of sounds/music. For example, classical music is often sorted in different musical sections (e.g. "Piano Concerto", "Weather - Hurricane").

##### TIT2
The _Title/Songname/Content description_ frame is the actual name of the piece (e.g. "Adagio", "Hurricane Donna").

##### TIT3
The _Subtitle/Description refinement_ frame is used for information directly related to the contents title (e.g. "Op. 16" or "Performed live at Wembley").

##### TKEY
The _Initial key_ frame contains the musical key in which the sound starts. It is represented as a string with a maximum length of three characters. The ground keys are represented with `"A"`,`"B"`,`"C"`,`"D"`,`"E"`, `"F"` and `"G"` and halfkeys represented with `"b"` and `"#"`. Minor is represented as `"m"`. Example `"Cbm"`. Off key is represented with an `"o"` only.

##### TLAN
The _Language(s)_ frame should contain the languages of the text or lyrics spoken or sung in the audio. The language is represented with three characters according to ISO-639-2. If more than one language is used in the text their language codes should follow according to their usage.

##### TLEN
The _Length_ frame contains the length of the audio file in milliseconds, represented as a numeric string.

##### TMED
The _Media type_ frame describes from which media the sound originated. This may be a text string or a reference to the predefined media types found in the list below. References are made within `"("` and `")"` and are optionally followed by a text refinement, e.g. `"(MC) with four channels"`. If a text refinement should begin with a `"("` character it should be replaced with `"(("` in the same way as in the [`TCON`](#tcon) frame. Predefined refinements is appended after the media type, e.g. `"(CD/A)"` or `"(VID/PAL/VHS)"`.

    DIG    Other digital media
      /A    Analog transfer from media

    ANA    Other analog media
      /WAC  Wax cylinder
      /8CA  8-track tape cassette

    CD     CD
      /A    Analog transfer from media
      /DD   DDD
      /AD   ADD
      /AA   AAD

    LD     Laserdisc
      /A     Analog transfer from media

    TT     Turntable records
      /33    33.33 rpm
      /45    45 rpm
      /71    71.29 rpm
      /76    76.59 rpm
      /78    78.26 rpm
      /80    80 rpm

    MD     MiniDisc
      /A    Analog transfer from media

    DAT    DAT
      /A    Analog transfer from media
      /1    standard, 48 kHz/16 bits, linear
      /2    mode 2, 32 kHz/16 bits, linear
      /3    mode 3, 32 kHz/12 bits, nonlinear, low speed
      /4    mode 4, 32 kHz/12 bits, 4 channels
      /5    mode 5, 44.1 kHz/16 bits, linear
      /6    mode 6, 44.1 kHz/16 bits, 'wide track' play

    DCC    DCC
      /A    Analog transfer from media

    DVD    DVD
      /A    Analog transfer from media

    TV     Television
      /PAL    PAL
      /NTSC   NTSC
      /SECAM  SECAM

    VID    Video
      /PAL    PAL
      /NTSC   NTSC
      /SECAM  SECAM
      /VHS    VHS
      /SVHS   S-VHS
      /BETA   BETAMAX

    RAD    Radio
      /FM   FM
      /AM   AM
      /LW   LW
      /MW   MW

    TEL    Telephone
      /I    ISDN

    MC     MC (normal cassette)
      /4    4.75 cm/s (normal speed for a two sided cassette)
      /9    9.5 cm/s
      /I    Type I cassette (ferric/normal)
      /II   Type II cassette (chrome)
      /III  Type III cassette (ferric chrome)
      /IV   Type IV cassette (metal)

    REE    Reel
      /9    9.5 cm/s
      /19   19 cm/s
      /38   38 cm/s
      /76   76 cm/s
      /I    Type I cassette (ferric/normal)
      /II   Type II cassette (chrome)
      /III  Type III cassette (ferric chrome)
      /IV   Type IV cassette (metal)

##### TOAL
The _Original album/movie/show title_ frame is intended for the title of the original recording (or source of sound), if for example the music in the file should be a cover of a previously released song.

##### TOFN
The _Original filename_ frame contains the preferred filename for the file, since some media doesn't allow the desired length of the filename. The filename is case sensitive and includes its suffix.

##### TOLY
The _Original lyricist(s)/text writer(s)_ frame is intended for the text writer(s) of the original recording, if for example the music in the file should be a cover of a previously released song. The text writers are seperated with the `"/"` character.

##### TOPE
The _Original artist(s)/performer(s)_ frame is intended for the performer(s) of the original recording, if for example the music in the file should be a cover of a previously released song. The performers are separated with the `"/"` character.

##### TORY
The _Original release year_ frame is intended for the year when the original recording, if for example the music in the file should be a cover of a previously released song, was released. The field is formatted as in the `"TYER"` frame.

##### TOWN
The _File owner/licensee_ frame contains the name of the owner or licensee of the file and it's contents.

##### TPE1
The _Lead artist(s)/Lead performer(s)/Soloist(s)/Performing group_ is used for the main artist(s). They are seperated with the `"/"` character.

##### TPE2
The _Band/Orchestra/Accompaniment_ frame is used for additional information about the performers in the recording.

##### TPE3
The _Conductor_ frame is used for the name of the conductor.

##### TPE4
The _Interpreted, remixed, or otherwise modified by_ frame contains more information about the people behind a remix and similar interpretations of another existing piece.

##### TPOS
The _Part of a set_ frame is a numeric string that describes which part of a set the audio came from. This frame is used if the source described in the [`TALB`](#talb) frame is divided into several mediums, e.g. a double CD. The value may be extended with a "/" character and a numeric string containing the total number of parts in the set. E.g. "1/2".

##### TPUB
The _Publisher_ frame simply contains the name of the label or publisher.

##### TRCK
The _Track number/Position in set_ frame is a numeric string containing the order number of the audio-file on its original recording. This may be extended with a `"/"` character and a numeric string containing the total number of tracks/elements on the original recording. E.g. `"4/9"`.

##### TRDA
The _Recording dates_ frame is a intended to be used as complement to the [`TYER`](#tyer), [`TDAT`](#tdat) and [`TIME`](#time) frames. E.g. `"4th-7th June, 12th June`" in combination with the [`TYER`](#tyer) frame.

##### TRSN
The _Internet radio station name_ frame contains the name of the internet radio station from which the audio is streamed.

##### TRSO
The _Internet radio station owner_ frame contains the name of the owner of the Internet radio station from which the audio is streamed.

##### TSIZ
The _Size_ frame contains the size of the audio file in bytes, excluding the ID3v2 tag, represented as a numeric string.

##### TSRC
The _ISRC_ frame should contain the International Standard Recording Code [ISRC] (12 characters).

##### TSSE
The _Software/Hardware and settings used for encoding_ frame includes the used audio encoder and its settings when the file was encoded. Hardware refers to hardware encoders, not the computer on which a program was run.

##### TYER
The _Year_ frame is a numeric string with a year of the recording. This frames is always four characters long (until the year 10000).


#### 4.2.2. User defined text information frame

This frame is intended for one-string text information concerning the audio file in a similar way to the other `T-frames`. The frame body consists of a description of the string, represented as a terminated string, followed by the actual string. There may be more than one `TXXX` frame in each tag, but only one with the same description.

    <Header for 'User defined text information frame', ID: "TXXX">
    text_encoding               $xx
    description                 <text_string_according_to_encoding> $00 (00)
    value                       <text_string_according_to_encoding>


### 4.3. URL link frames

With these frames dynamic data such as web pages with touring information, price information or plain ordinary news can be added to the tag. There may only be one URL [URL] link frame of its kind in an tag, except when stated otherwise in the frame description. If the text string is followed by a termination (`$00 (00)`) all the following information should be ignored and not be displayed. All URL link frame identifiers begins with `"W"`. Only URL link frame identifiers begins with `"W"`. All URL link frames have the following format:

    <Header for 'URL link frame', ID: "W000" - "WZZZ", excluding "WXXX" described in 4.3.2.>
    url                         <text_string>

#### 4.3.1. URL link frames - details

##### WCOM
The _Commercial information_ frame is a URL pointing at a web page with information such as where the album can be bought. There may be more than one `WCOM` frame in a tag, but not with the same content.

##### WCOP
The _Copyright/Legal information_ frame is a URL pointing at a web page where the terms of use and  ownership of the file is described.

##### WOAF
The _Official audio file web page_ frame is a URL pointing at a file specific web page.

##### WOAR
The _Official artist/performer web page_ frame is a URL pointing at the artists official web page. There may be more than one `WOAR` frame in a tag if the audio contains more than one performer, but not with the same content.

##### WOAS
The _Official audio source web page_ frame is a URL pointing at the official web page for the source of the audio file, e.g. a movie.

##### WORS
The _Official Internet radio station homepage_ contains a URL pointing at the homepage of the Internet radio station.

##### WPAY
The _Payment_ frame is a URL pointing at a web page that will handle the process of paying for this file.

##### WPUB
The _Publishers official web page_ frame is a URL pointing at the official web page for the publisher.


#### 4.3.2. User defined URL link frame

This frame is intended for URL [URL] links concerning the audio file in a similar way to the other `W-frames`. The frame body consists of a description of the string, represented as a terminated string, followed by the actual URL. The URL is always encoded with ISO-8859-1 [ISO-8859-1]. There may be more than one `WXXX` frame in each tag, but only one with the same description.

    <Header for 'User defined URL link frame', ID: "WXXX">
    text_encoding               $xx
    description                 <text_string_according_to_encoding> $00 (00)
    url                         <text_string>


### 4.4. Involved people list

Since there might be a lot of people contributing to an audio file in various ways, such as musicians and technicians, the _Text information frames_ are often insufficient to list everyone involved
in a project. The _Involved people list_ is a frame containing the names of those involved, and how they were involved. The body simply contains a terminated string with the involvement directly followed by a terminated string with the involvee followed by a new involvement and so on. There may only be one `IPLS` frame in each tag.

    <Header for 'Involved people list', ID: "IPLS">
    text_encoding               $xx
    people_list_strings         <text_strings_according_to_encoding>


### 4.5. Music CD identifier

This frame is intended for music that comes from a CD, so that the CD can be identified in databases such as the CDDB [CDDB]. The frame consists of a binary dump of the Table Of Contents, TOC, from the CD, which is a header of 4 bytes and then 8 bytes/track on the CD plus 8 bytes for the 'lead out' making a maximum of 804 bytes. The offset to the beginning of every track on the CD should be described with a four bytes absolute CD-frame address per track, and not with absolute time. This frame requires a present and valid [`TRCK`](#trck) frame, even if the CD's only got one track. There may only be one `MCDI` frame in each tag.

    <Header for 'Music CD identifier', ID: "MCDI">
    cd_toc                      <binary data>


### 4.6. Event timing codes

This frame allows synchronization with key events in a song or sound. The header is:

    <Header for 'Event timing codes', ID: "ETCO">
    time_stamp_format           $xx

Where `time_stamp_format` is:

- `$01`, absolute time, 32 bit sized, using MPEG [MPEG] frames as unit,
- `$02`, absolute time, 32 bit sized, using milliseconds as unit.

Absolute time means that every stamp contains the time from the beginning of the file.
Followed by a list of key events in the following format:

    type_of_event               $xx
    time_stamp                  $xx (xx ...)

The `time_stamp` is set to zero if directly at the beginning of the sound or after the previous event. All events should be sorted in chronological order. The `type_of_event` is as follows:

- `$00`, padding (has no meaning),
- `$01`, end of initial silence,
- `$02`, intro start,
- `$03`, mainpart start,
- `$04`, outro start,
- `$05`, outro end,
- `$06`, verse start,
- `$07`, refrain start,
- `$08`, interlude start,
- `$09`, theme start,
- `$0A`, variation start,
- `$0B`, key change,
- `$0C`, time change,
- `$0D`, momentary unwanted noise (Snap, Crackle & Pop),
- `$0E`, sustained noise,
- `$0F`, sustained noise end,
- `$10`, intro end,
- `$11`, mainpart end,
- `$12`, verse end,
- `$13`, refrain end,
- `$14`, theme end,
- `$15-$DF`, reserved for future use,
- `$E0-$EF`, not predefined sync 0-F,
- `$F0-$FC`, reserved for future use,
- `$FD`, audio end (start of silence),
- `$FE`, audio file ends,
- `$FF`, one more byte of events follows (all the following bytes with the value `$FF` have the same function).

Terminating the start events such as "intro start" is not required. The _Not predefined sync's_ (`$E0-EF`) are for user events. You might want to synchronize your music to something, like setting of an explosion on-stage, turning on your screen saver etc.

There may only be one `ETCO` frame in each tag.


### 4.7. MPEG location lookup table

To increase performance and accuracy of jumps within a MPEG [MPEG] audio file, frames with time codes in different locations in the file might be useful. The ID3v2 frame includes references that the software can use to calculate positions in the file. After the frame header is a descriptor of how much the `frame_counter` should increase for every reference. If this value is two then the first reference points out the second frame, the 2nd reference the 4th frame, the 3rd reference the 6th frame etc. In a similar way the `bytes_between_reference` and `milliseconds_between_reference` points out bytes and milliseconds respectively.

Each reference consists of two parts; a certain number of bits, as defined in `bits_for_bytes_deviation`, that describes the difference between what is said in 'bytes between reference' and the reality and a certain number of bits, as defined in `bits_for_milliseconds_deviation`, that describes the difference between what is said in `milliseconds_between_reference` and the reality. The number of bits in every reference, i.e. `bits_for_bytes_deviation + bits_for_milliseconds_deviation`, must be a multiple of four. There may only be one `MLLT` frame in each tag.

    <Header for 'Location lookup table', ID: "MLLT">
    MPEG_frames_between_reference      $xx xx
    bytes_between_reference            $xx xx xx
    milliseconds_between_reference     $xx xx xx
    bits_for_bytes_deviation           $xx
    bits_for_milliseconds_deiationv    $xx

Then for every reference the following data is included;

    deviation_in_bytes          %xxx....
    deviation_in_milliseconds   %xxx....


### 4.8. Synchronized tempo codes

For a more accurate description of the tempo of a musical piece this frame might be used. After the header follows one byte describing which time stamp format should be used. Then follows one or more tempo codes. Each tempo code consists of one tempo part and one time part. The tempo is in BPM described with one or two bytes. If the first byte has the value `$FF`, one more byte follows, which is added to the first giving a range from 2 - 510 BPM, since `$00` and `$01` is reserved. `$00` is used to describe a beat-free time period, which is not the same as a music-free time period. `$01` is used to indicate one single beat-stroke followed by a beat-free period.

The tempo descriptor is followed by a time stamp. Every time the tempo in the music changes, a tempo descriptor may indicate this for the player. All tempo descriptors should be sorted in chronological order. The first beat-stroke in a time-period is at the same time as the beat description occurs. There may only be one "SYTC" frame in each tag.

    <Header for 'Synchronised tempo codes', ID: "SYTC">
    time_stamp_format           $xx
    tempo_data                  <binary_data>

Where time stamp format is:

- `$01`, absolute time, 32 bits sized, using MPEG [MPEG] frames as unit,
- `$02`, absolute time, 32 bits sized, using milliseconds as unit.

Absolute time means that every stamp contains the time from the beginning of the file.


### 4.9. Unsychronized lyrics/text transcription

This frame contains the lyrics of the song or a text transcription of other vocal activities. The head includes an encoding descriptor and a content descriptor. The body consists of the actual text. The _Content descriptor_ is a terminated string. If no descriptor is entered, _Content descriptor_ is `$00 (00)` only. Newline characters are allowed in the text. There may be more than one _Unsynchronized lyrics/text transcription_ frame in each tag, but only one with the same language and content descriptor.

    <Header for 'Unsynchronised lyrics/text transcription', ID: "USLT">
    text_encoding               $xx
    language                    $xx xx xx
    content_descriptor          <text_string_according_to_encoding> $00 (00)
    lyrics_text                 <full_text_string_according_to_encoding>


### 4.10. Synchronized lyrics/text

This is another way of incorporating the words, said or sung lyrics, in the audio file as text, this time, however, in sync with the audio. It might also be used to describing events e.g. occurring on a stage or on the screen in sync with the audio. The header includes a content descriptor, represented with as terminated text string. If no descriptor is entered, _Content descriptor_ is `$00 (00)` only.

    <Header for 'Synchronised lyrics/text', ID: "SYLT">
    text_encoding               $xx
    language                    $xx xx xx
    time_stamp_format           $xx
    content_type                $xx
    content_descriptor          <text_string_according_to_encoding> $00 (00)

`encoding` is:

- `$00`, ISO-8859-1 [ISO-8859-1] character set is used => `$00` is sync identifier,
- `$01`, Unicode [UNICODE] character set is used => `$00 00` is sync identifier.

`content_type` is:

- `$00`, is other,
- `$01`, is lyrics,
- `$02`, is text transcription,
- `$03`, is movement/part name (e.g. "Adagio"),
- `$04`, is events (e.g. "Don Quijote enters the stage"),
- `$05`, is chord (e.g. "Bb F Fsus"),
- `$06`, is trivia/'pop up' information.

`time_stamp_format` is:

- `$01`, absolute time, 32 bit sized, using MPEG [MPEG] frames as unit,
- `$02`, absolute time, 32 bit sized, using milliseconds as unit.

Absolute time means that every stamp contains the time from the beginning of the file.

The text that follows the frame header differs from that of the unsynchronized lyrics/text transcription in one major way. Each syllable (or whatever size of text is considered to be convenient by the encoder) is a null terminated string followed by a time stamp denoting where in the sound file it belongs. Each sync thus has the following structure:

    terminated_text_to_be_synced (typically a syllable)
    sync_identifier (terminator to above string)   $00 (00)
    time_stamp                                     $xx (xx ...)

The `time_stamp` is set to zero or the whole sync is omitted if located directly at the beginning of the sound. All time stamps should be sorted in chronological order. The sync can be considered as a validator of the subsequent string.

Newline `($0A)` characters are allowed in all `SYLT` frames and should be used after every entry (name, event etc.) in a frame with the content type `$03 - $04`.

A few considerations regarding whitespace characters: Whitespace separating words should mark the beginning of a new word, thus occurring in front of the first syllable of a new word. This is also valid for new line characters. A syllable followed by a comma should not be broken apart with a sync (both the syllable and the comma should be before the sync).

An example: The "USLT" passage

    "Strangers in the night" $0A "Exchanging glances"

would be "SYLT" encoded as:

    "Strang" $00 xx xx "ers" $00 xx xx " in" $00 xx xx " the" $00 xx xx
    " night" $00 xx xx 0A "Ex" $00 xx xx "chang" $00 xx xx "ing" $00 xx xx "glan" $00 xx xx "ces" $00 xx xx

There may be more than one `SYLT` frame in each tag, but only one with the same language and content descriptor.


### 4.11. Comments

This frame is intended for any kind of full text information that does not fit in any other frame. It consists of a frame header followed by encoding, language and content descriptors and is ended with the actual comment as a text string. Newline characters are allowed in the comment text string. There may be more than one comment frame in each tag, but only one with the same language and content descriptor.

    <Header for 'Comment', ID: "COMM">
    text_encoding               $xx
    language                    $xx xx xx
    short_content_description   <text string according to encoding> $00 (00)
    the_actual_text             <full text string according to encoding>


### 4.12. Relative volume adjustment

This is a more subjective function than the previous ones. It allows the user to say how much he wants to increase/decrease the volume on each channel while the file is played. The purpose is to be able to align all files to a reference volume, so that you don't have to change the volume constantly. This frame may also be used to balance adjust the audio. If the volume peak levels are known then this could be described with the 'Peak volume right' and 'Peak volume left' field. If Peak volume is not known these fields could be left zeroed or, if no other data follows, be completely omitted. There may only be one `RVAD` frame in each tag.

    <Header for 'Relative volume adjustment', ID: "RVAD">
    increment_decrement                %00xxxxxx
    bits_used_for_volume_description.  $xx
    relative_volume_change_right       $xx xx (xx ...)
    relative_volume_change_left        $xx xx (xx ...)
    peak_volume_right                  $xx xx (xx ...)
    peak_volume_left                   $xx xx (xx ...)

In the `increment_decrement` field bit 0 is used to indicate the right channel and bit 1 is used to indicate the left channel. 1 is increment and 0 is decrement.

The `bits_used_for_volume_description` field is normally `$10` (16 bits) for MPEG 2 layer I, II and III [MPEG] and MPEG 2.5. This value may not be `$00`. The volume is always represented with whole bytes, padded in the beginning (highest bits) when `bits_used_for_volume_description` is not a multiple of eight.

This data block is then optionally followed by a volume definition for the left and right back channels. If this information is appended to the frame the first two channels will be treated as front channels. In the increment/decrement field bit 2 is used to indicate the right back channel and bit 3 for the left back channel.

    relative_volume_change_right_back  $xx xx (xx ...)
    relative_volume_change_left_back   $xx xx (xx ...)
    peak_volume_right_back             $xx xx (xx ...)
    peak_volume_left_back              $xx xx (xx ...)

If the center channel adjustment is present the following is appended to the existing frame, after the left and right back channels. The center channel is represented by bit 4 in the `increment_decrement` field.

    relative_volume_change_center      $xx xx (xx ...)
    peak_volume_center                 $xx xx (xx ...)

If the bass channel adjustment is present the following is appended to the existing frame, after the center channel. The bass channel is represented by bit 5 in the increase_decrease field.

    relative_volume_change_bass        $xx xx (xx ...)
    peak_volume_bass                   $xx xx (xx ...)


### 4.13. Equalization

This is another subjective, alignment frame. It allows the user to predefine an equalization curve within the audio file. There may only be one `EQUA` frame in each tag.

    <Header of 'Equalisation', ID: "EQUA">
    adjustment_bits             $xx

The `adjustment_bits` field defines the number of bits used for representation of the adjustment. This is normally `$10` (16 bits) for MPEG 2 layer I, II and III [MPEG] and MPEG 2.5. This value may not be  `$00`.

This is followed by 2 bytes + (`adjustment_bits` rounded up to the nearest byte) for every equalization band in the following format, giving a frequency range of 0 - 32767Hz:

    increment_decrement         %x (MSB of the Frequency)
    frequency                   (lower 15 bits)
    adjustment                  $xx (xx ...)

The `increment_decrement` bit is 1 for increment and 0 for decrement. The equalization bands should be ordered increasingly with reference to frequency. All frequencies don't have to be declared. The equalization curve in the reading software should be interpolated between the values in this frame. Three equal adjustments for three subsequent frequencies. A frequency should only be described once in the frame.


### 4.14. Reverb

Yet another subjective one. You may here adjust echoes of different kinds. `reverb_left` and `reverb_right` are delay between every bounce in ms. `reverb_bounces_left` and `reverb_bounces_right` are numbers of bounces that should be made. `$FF` equals an infinite number of bounces. Feedback are the amount of volume that should be returned to the next echo bounce. `$00` is 0%, `$FF` is 100%. If this value were `$7F`, there would be 50% volume reduction on the first bounce, 50% of that on the second and so on. Left to left means the sound from the left bounce to be played in the left speaker, while left to right means sound from the left bounce to be played in the right speaker.`premix_left_to _right` is the amount of left sound to be mixed in the right before any reverb is applied, where `$00` is 0% and `$FF` is 100%. `premix_right_to_left` does the same thing, but right to left. Setting both premix to `$FF` would result in a mono output (if the reverb is applied symmetric). There may only be one `RVRB` frame in each tag.

    <Header for 'Reverb', ID: "RVRB">
    reverb_left                     $xx xx
    reverb_right                    $xx xx
    reverb_bounces_left             $xx
    reverb_bounces_right            $xx
    reverb_feedback_left_to_left    $xx
    reverb_feedback_left_to_right   $xx
    reverb_feedback_right_to_right  $xx
    reverb_feedback_right_to_left   $xx
    premix_left_to_right            $xx
    premix_right_to_left            $xx


### 4.15. Attached picture

This frame contains a picture directly related to the audio file. Image format is the MIME type and subtype [MIME] for the image. In the event that the MIME media type name is omitted, "image/" will be implied. The `image/png` [PNG] or `image/jpeg` [JFIF] picture format should be used when interoperability is wanted. Description is a short description of the picture, represented as a terminated text string. The description has a maximum length of 64 characters, but may be empty. There may be several pictures attached to one file, each in their individual `APIC` frame, but only one with the same content descriptor. There may only be one picture with the picture type declared as picture type `$01` and `$02` respectively. There is the possibility to put only a link to the image file by using the 'MIME type' `-->` and having a complete URL [URL] instead of picture data. The use of linked files should however be used sparingly since there is the risk of separation of files.

    <Header for 'Attached picture', ID: "APIC">
    text_encoding               $xx
    mime_type                   <text_string> $00
    picture_type                $xx
    description                 <text_string_according_to_encoding> $00 (00)
    picture_data                <binary_data>

`picture_type` is:

- `$00`, other,
- `$01`, 32x32 pixels 'file icon' (PNG only),
- `$02`, other file icon,
- `$03`, cover (front),
- `$04`, cover (back),
- `$05`, leaflet page,
- `$06`, media (e.g. lable side of CD),
- `$07`, lead artist/lead performer/soloist,
- `$08`, artist/performer,
- `$09`, conductor,
- `$0A`, band/Orchestra,
- `$0B`, composer,
- `$0C`, lyricist/text writer,
- `$0D`, recording Location,
- `$0E`, during recording,
- `$0F`, during performance,
- `$10`, movie/video screen capture,
- `$11`, a bright coloured fish,
- `$12`, illustration,
- `$13`, band/artist logotype,
- `$14`, publisher/Studio logotype.


### 4.16. General encapsulated object

In this frame any type of file can be encapsulated. After the header, `frame_size` and `text_encoding` follows `mime_type` [MIME] represented as a terminated string encoded with ISO 8859-1 [ISO-8859-1]. The filename is case sensitive and is encoded as `text_encoding`. Then follows a content description as terminated string, encoded as `text_encoding`. The last thing in the frame is the actual object. The first two strings may be omitted, leaving only their terminations. `mime_type` is always an ISO-8859-1 text string. There may be more than one `GEOB` frame in each tag, but only one with the same content descriptor.

    <Header for 'General encapsulated object', ID: "GEOB">
    text_encoding               $xx
    mime_type                   <text_string> $00
    filename                    <text_string_according_to_encoding> $00 (00)
    content_description         <text_string_according_to_encoding> $00 (00)
    encapsulated_object         <binary_data>


### 4.17. Play counter

This is simply a counter of the number of times a file has been played. The value is increased by one every time the file begins to play. There may only be one `PCNT` frame in each tag. When the counter reaches all one's, one byte is inserted in front of the counter thus making the counter eight bits bigger. The counter must be at least 32-bits long to begin with.

    <Header for 'Play counter', ID: "PCNT">
    counter                     $xx xx xx xx (xx ...)


### 4.18. Popularimeter

The purpose of this frame is to specify how good an audio file is. Many interesting applications could be found to this frame such as a playlist that features better audio files more often than others or it could be used to profile a person's taste and find other _good_ files by comparing people's profiles. The frame is very simple. It contains the email address to the user, one rating byte and a four byte play counter, intended to be increased with one for every time the file is played. The email is a terminated string. The rating is 1-255 where 1 is worst and 255 is best. 0 is unknown. If no personal counter is wanted it may be omitted.  When the counter reaches all one's, one byte is inserted in front of the counter thus making the counter eight bits bigger in the same away as the play counter [`PCNT`](#417-play-counter). There may be more than one `POPM` frame in each tag, but only one with the same email address.

    <Header for 'Popularimeter', ID: "POPM">
    email_to_user               <text_string> $00
    rating                      $xx
    counter                     $xx xx xx xx (xx ...)


### 4.19. Recommended buffer size

Sometimes the server from which a audio file is streamed is aware of transmission or coding problems resulting in interruptions in the audio stream. In these cases, the size of the buffer can be
recommended by the server using this frame. If the `embedded_info_flag` is true (1) then this indicates that an ID3 tag with the maximum size described in `buffer_size` may occur in the audio stream. In such case the tag should reside between two MPEG [MPEG] frames, if the audio is MPEG encoded. If the position of the next tag is known, `offset_to_next_tag` may be used. The offset is calculated from the end of tag in which this frame resides to the first byte of the header in the next. This field may be omitted. Embedded tags are generally not recommended since this could render unpredictable behavior from present software/hardware.

For applications like streaming audio it might be an idea to embed tags into the audio stream though. If the clients connects to individual connections like HTTP and there is a possibility to begin every transmission with a tag, then this tag should include a `recommended_buffer_size` frame. If the client is connected to a arbitrary point in the stream, such as radio or multicast, then the `recommended_buffer_size` frame should be included in every tag. Every tag that is picked up after the initial/first tag is to be considered as an update of the previous one. E.g. if there is a
[`TIT2`](#tit2) frame in the first received tag and one in the second tag, then the first should be replaced with the second.

The `buffer_size` should be kept to a minimum. There may only be one `RBUF` frame in each tag.

    <Header for 'Recommended buffer size', ID: "RBUF">
    buffer_size                 $xx xx xx
    embedded_info_flag          %0000000x
    offset_to_next_tag          $xx xx xx xx


### 4.20. Audio encryption

This frame indicates if the actual audio stream is encrypted, and by whom. Since standardization of such encryption scheme is beyond this document, all `AENC` frames begin with a terminated string with a URL containing an email address, or a link to a location where an email address can be found, that belongs to the organization responsible for this specific encrypted audio file. Questions regarding the encrypted audio should be sent to the email address specified. If a `$00` is found directly after the `frame_size` and the audio file indeed is encrypted, the whole file may be considered useless.

After the `owner_identifier`, a pointer to an unencrypted part of the audio can be specified. The `preview_start` and `preview_length` is described in frames. If no part is unencrypted, these fields should be left zeroed. After the `preview_length` field follows optionally a data block required for decryption of the audio. There may be more than one `AENC` frames in a tag, but only one with the same `owner_identifier`.

    <Header for 'Audio encryption', ID: "AENC">
    owner_identifier            <text_string> $00
    preview_start               $xx xx
    preview_length              $xx xx
    encryption_info             <binary_data>


### 4.21. Linked information

To keep space waste as low as possible this frame may be used to link information from another ID3v2 tag that might reside in another audio file or alone in a binary file. It is recommended that this method is only used when the files are stored on a CD-ROM or other circumstances when the risk of file separation is low. The frame contains a frame identifier, which is the frame that should be linked into this tag, a `url` [URL] field, where a reference to the file where the frame is given, and additional `id` data, if needed. Data should be retrieved from the first tag found in the file to which this link points. There may be more than one  `LINK` frame in a tag, but only one with the same contents. A linked frame is to be considered as part of the tag and has the same restrictions as if it was a physical part of the tag (i.e. only one [`RVRB`](#414-reverb) frame allowed, whether it's linked or not).

    <Header for 'Linked information', ID: "LINK">
    frame_id                    $xx xx xx
    url                         <text string> $00
    additional_id_data          <text_string(s)>

Frames that may be linked and need no additional data are [`IPLS`](#44-involved-people-list), [`MCDI`](#45-music-cd-identifier), [`ETCO`](#46-event-timing-code), [`MLLT`](#47-mpeg-location-lookup-table), [`SYTC`](#48-synchronized-tempo-codes), [`RVAD`](#412-relative-volume-adjustment), [`EQUA`](#413-equalization), [`RVRB`](#414-reverb), [`RBUF`](#419-recommended-buffer-size), the text information frames (`T-Frames`) and the URL link frames (`U-Frames`).

The [`TXXX`](#422-user-defined-text-information-frame), [`APIC`](#415-attached-picture), [`GEOB`](#416-general-encapsulated-object) and  [`AENC`](#421-audio-encryption) frames may be linked with the content descriptor as `additional_id_data`.

The [`COMM`](#411-comments), [`SYLT`](#410-synchronized-lyrictext) and [`USLT`](#49-unsynchronized-lyrictext-transcription) frames may be linked with three bytes of language descriptor directly followed by a content descriptor as `additional_id_data`.


### 4.22. Position synchronization frame

This frame delivers information to the listener of how far into the audio stream he picked up; in effect, it states the time offset of the first frame in the stream. The frame layout is:

    <Head for 'Position synchronisation', ID: "POSS">
    time_stamp_format           $xx
    position                    $xx (xx ...)

Where `time_stamp_ format` is:

- `$01`, absolute time, 32 bit sized, using MPEG frames as unit,
- `$02`, absolute time, 32 bit sized, using milliseconds as unit.

and position is where in the audio the listener starts to receive, i.e. the beginning of the next frame. If this frame is used in the beginning of a file the value is always 0. There may only be one `POSS` frame in each tag.


### 4.23. Terms of use frame

This frame contains a brief description of the terms of use and ownership of the file. More detailed information concerning the legal terms might be available through the [`WCOP`](#431-copyright-Legal-information) frame. Newlines are allowed in the text. There may only be one `USER` frame in a tag.

    <Header for 'Terms of use frame', ID: "USER">
    text_encoding               $xx
    language                    $xx xx xx
    the_actual_text             <text_string_according_to_encoding>


### 4.24. Ownership frame

The ownership frame might be used as a reminder of a made transaction or, if signed, as proof. Note that the [`USER`](#423-terms-of-use)" and [`TOWN`](#421-TOWN) frames are good to use in conjunction with this one. The frame begins, after the frame ID, size and encoding fields, with a `price_paye` field. The first three characters of this field contains the currency used for the transaction, encoded according to ISO 4217 [ISO-4217] alphabetic currency code. Concatenated to this is the actual price payed, as a numerical string using `"."` as the decimal separator. Next is an 8 characters date string (`YYYYMMDD`) followed by a string with the name of the seller as the last field in the frame. There may only be one `OWNE` frame in a tag.

    <Header for 'Ownership frame', ID: "OWNE">
    text_encoding               $xx
    price_payed                 <text_string> $00
    date_of_purchase            <text_string>
    seller                      <text_string_according_to_encoding>


### 4.25. Commercial frame

This frame enables several competing offers in the same tag by bundling all needed information. That makes this frame rather complex but it's an easier solution than if one tries to achieve the same result with several frames. The frame begins, after the `frame_id                    size and encoding fields, with a price string field. A price is constructed by one three character currency code, encoded according                 to ISO 4217 [ISO-4217] alphabetic currency code, followed by a numerical value where `"."` is used as decimal separator. In the pr              ice string several prices may be concatenated, separated by a `"/"` character, but there may only be one currency of each type.

The price string is followed by an 8 character date string in the format `YYYYMMDD`, describing for how long the price is valid. After that is a contact URL, with which the user can contact the seller, followed by a one byte `received_as` field. It describes how the audio is delivered when bought according to the following list:

- `$00`, other,
- `$01`, standard CD album with other songs,
- `$02`, compressed audio on CD,
- `$03`, file over the Internet,
- `$04`, stream over the Internet,
- `$05`, as note sheets,
- `$06`, as note sheets in a book with other sheets,
- `$07`, music on other media,
- `$08`, non-musical merchandise.

Next follows a terminated string with the name of the seller followed by a terminated string with a short description of the product. The last thing is the ability to include a company logotype. The first of them is the `picture_mime_type` field containing information about which picture format is used. In the event that the MIME media type name is omitted, `"image/"` will be implied. Currently only `"image/png"` and `"image/jpeg"` are allowed. This format string is followed by the binary picture data. This two last fields may be omitted if no picture is to attach.

    <Header for 'Commercial frame', ID: "COMR">
    text_encoding               $xx
    price_string                <text_string> $00
    valid_until                 <text_string>
    contact_url                 <text_string> $00
    received_as                 $xx
    name_of_seller              <text_string_according_to_encoding> $00 (00)
    description                 <text_string_according_to_encoding> $00 (00)
    picture_mime_type           <string> $00
    seller_logo                 <binary_data>


### 4.26. Encryption method registration

To identify with which method a frame has been encrypted the encryption method must be registered in the tag with this frame. The `owner_identifier` is a null-terminated string with a URL [URL] containing an email address, or a link to a location where an email address can be found, that belongs to the organization responsible for this specific encryption method. Questions regarding the encryption method should be sent to the indicated email address. The `method_symbol` contains a value that is associated with this method throughout the whole tag. Values below `$80` are reserved. The `method_symbol` may optionally be followed by encryption specific data. There may be several `ENCR` frames in a tag but only one containing the same symbol and only one containing the same owner identifier. The method must be used somewhere in the tag (see _section [3.3.1](#331-frame-header-flags)_, flag j for more information).

    <Header for 'Encryption method registration', ID: "ENCR">
    owner_identifier            <text_string> $00
    method_symbol               $xx
    encryption_data             <binary_data>


### 4.27. Group identification registration

This frame enables grouping of otherwise unrelated frames. This can be used when some frames are to be signed. To identify which frames belongs to a set of frames a group identifier must be registered in the tag with this frame. The `owner_identifier` is a null-terminated string with a URL [URL] containing an email address, or a link to a location where an email address can be found, that belongs to the organization responsible for this grouping. Questions regarding the grouping should be sent to the indicated email address. The `group_symbol` contains a value that associates the frame with this group throughout the whole tag. Values below `$80` are reserved. The `group_symbol` may optionally be followed by some group specific data, e.g. a _digital signature_. There may be several `GRID` frames in a tag but only one containing the same symbol and only one containing the same owner identifier. The group symbol must be used somewhere in the tag (see _section [3.3.1](#331-frame-header-flags)_, flag j for more information).

    <Header for 'Group ID registration', ID: "GRID">
    owner_identifier            <text_string> $00
    group_symbol                $xx
    group_dependent_data        <binary_data>


### 4.28. Private frame

This frame is used to contain information from a software producer that its program uses and does not fit into the other frames. The frame consists of an `owner_identifier` string and the binary data. The `owner_identifier` is a null-terminated string with a URL [URL] containing an email address, or a link to a location where an email address can be found, that belongs to the organization responsible for the frame. Questions regarding the frame should be sent to the indicated email address. The tag may contain more than one `PRIV` frame but only with different contents. It is recommended to keep the number of `PRIV` frames as low as possible.

    <Header for 'Private frame', ID: "PRIV">
    owner_identifier            <text_string> $00
    the_private_data            <binary_data>



5. The unsynchronization scheme
-------------------------------

The only purpose of the _unsynchronization scheme_ is to make the ID3v2 tag as compatible as possible with existing software. There is no use in unsynchronizing tags if the file is only to be processed by new software. Unsynchronization may only be made with MPEG 2 layer I, II and III and MPEG 2.5 files.

Whenever a false synchronization is found within the tag, one zeroed byte is inserted after the first false synchronization byte. The format of a correct sync that should be altered by ID3 encoders is as follows:

        %11111111 111xxxxx

And should be replaced with:

        %11111111 00000000 111xxxxx

This has the side effect that all `$FF 00` combinations have to be altered, so they won't be affected by the decoding process. Therefore all the `$FF 00` combinations have to be replaced with the `$FF 00 00` combination during the unsynchronization.

To indicate usage of the unsynchronization, the first bit in `ID3_flags` should be set. This bit should only be set if the tag contains a, now corrected, false synchronization. The bit should only be clear if the tag does not contain any false synchronizations.

Do bear in mind, that if a compression scheme is used by the encoder, the unsynchronization scheme should be applied **_afterwards_**. When decoding a compressed, unsynchronized file, the unsynchronization scheme should be parsed first, decompression afterwards.

If the last byte in the tag is `$FF`, and there is a need to eliminate false `synchronisations` in the tag, at least one byte of padding should be added.



6. Copyright
------------

Copyright (C) Martin Nilsson 1998. All Rights Reserved.

This document and translations of it may be copied and furnished to others, and derivative works that comment on or otherwise explain it or assist in its implementation may be prepared, copied, published and distributed, in whole or in part, without restriction of any kind, provided that a reference to this document is included on all such copies and derivative works. However, this document itself may not be modified in any way and reissued as the original document.

The limited permissions granted above are perpetual and will not be revoked.

This document and the information contained herein is provided on an "AS IS" basis and THE AUTHORS DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTY THAT THE USE OF THE INFORMATION HEREIN WILL NOT INFRINGE ANY RIGHTS OR ANY IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.



7. References
--------------

1. <a name="1"></a> [CDDB] Compact Disc Data Base. http://www.cddb.com

2. <a name="2"></a> [ID3v2] Martin Nilsson, "ID3v2 informal standard". http://www.id3.org/id3v2-00.txt

3. <a name="3"></a> [ISO-639-2] ISO/FDIS 639-2. Codes for the representation of names of languages, Part 2: Alpha-3 code. Technical committee / subcommittee: TC 37 / SC 2

4. <a name="4"></a> [ISO-4217] ISO 4217:1995. Codes for the representation of currencies and funds. Technical committee / subcommittee: TC 68

5. <a name="5"></a> [ISO-8859-1] ISO/IEC DIS 8859-1. 8-bit single-byte coded graphic character sets, Part 1: Latin alphabet No. 1. Technical committee / subcommittee: JTC 1 / SC 2

6. <a name="6"></a> [ISRC] ISO 3901:1986 International Standard Recording Code (ISRC). Technical committee / subcommittee: TC 46 / SC 9

7. <a name="7"></a> [JFIF] JPEG File Interchange Format, version 1.02. http://www.w3.org/Graphics/JPEG/jfif.txt

8. <a name="8"></a> [MIME] Freed, N.  and N. Borenstein,  "Multipurpose Internet Mail Extensions (MIME) Part One: Format of Internet Message Bodies", RFC 2045, November 1996. ftp://ftp.isi.edu/in-notes/rfc2045.txt

9. <a name="9"></a> [MPEG] ISO/IEC 11172-3:1993. Coding of moving pictures and associated audio for digital storage media at up to about 1,5 Mbit/s, Part 3: Audio. Technical committee / subcommittee: JTC 1 / SC 29 and ISO/IEC 13818-3:1995  
Generic coding of moving pictures and associated audio information, Part 3: Audio. Technical committee/subcommittee: JTC 1 / SC 29 and ISO/IEC DIS 13818-3  
Generic coding of moving pictures and associated audio information, Part 3: Audio (Revision of ISO/IEC 13818-3:1995)

10. <a name="10"></a> [PNG] Portable Network Graphics, version 1.0. http://www.w3.org/TR/REC-png-multi.html

11. <a name="11"></a> [UNICODE] ISO/IEC 10646-1:1993. Universal Multiple-Octet Coded Character Set (UCS), Part 1: Architecture and Basic Multilingual Plane. Technical committee / subcommittee: JTC 1 / SC 2. http://www.unicode.org

12. <a name="12"></a> [URL] T. Berners-Lee, L. Masinter & M. McCahill, "Uniform Resource Locators (URL).", RFC 1738, December 1994. ftp://ftp.isi.edu/in-notes/rfc1738.txt

13. <a name="13"></a> [ZLIB] P. Deutsch, Aladdin Enterprises & J-L. Gailly, "ZLIB Compressed Data Format Specification version 3.3", RFC 1950, May 1996.  ftp://ftp.isi.edu/in-notes/rfc1950.txt



Appendix A
----------

The following genres is defined in ID3v1:

-  0=Blues
-  1=Classic Rock
-  2=Country,
-  3=Dance,
-  4=Disco,
-  5=Funk,
-  6=Grunge,
-  7=ip-Hop,
-  8=Jazz,
-  9=Metal,
- 10=New Age,
- 11=Oldies,
- 12=Other,
- 13=Pop,
- 14=R&B,
- 15=Rap,
- 16=Reggae,
- 17=Rock,
- 18=Techno,
- 19=Industrial,
- 20=Alternative,
- 21=Ska,
- 22=Death Metal,
- 23=Pranks,
- 24=Soundtrack,
- 25=Euro-Techno,
- 26=Ambient,
- 27=Trip-Hop,
- 28=Vocal,
- 29=Jazz+Funk,
- 30=Fusion,
- 31=Trance,
- 32=Classical,
- 33=Instrumental,
- 34=Acid,
- 35=House,
- 36=Game,
- 37=Sound Clip,
- 38=Gospel,
- 39=Noise,
- 40=AlternRock,
- 41=Bass,
- 42=Soul,
- 43=Punk,
- 44=Space,
- 45=Meditative,
- 46=Instrumental Pop,
- 47=Instrumental Rock,
- 48=Ethnic,
- 49=Gothic,
- 50=Darkwave,
- 51=Techno-Industrial,
- 52=Electronic,
- 53=Pop-Folk,
- 54=Eurodance,
- 55=Dream,
- 56=Southern Rock,
- 57=Comedy,
- 58=Cult,
- 59=Gangsta,
- 60=Top 40,
- 61=Christian Rap,
- 62=Pop/Funk,
- 63=Jungle,
- 64=Native American,
- 65=Cabaret,
- 66=New Wave,
- 67=Psychadelic,
- 68=Rave,
- 69=Showtunes,
- 70=Trailer,
- 71=Lo-Fi,
- 72=Tribal,
- 73=Acid Punk,
- 74=Acid Jazz,
- 75=Polka,
- 76=Retro,
- 77=Musical,
- 78=Rock & Roll,
- 79=Hard Rock.

The following genres are Winamp extensions:

-  80=Folk,
-  81=Folk-Rock,
-  82=National Folk,
-  83=Swing,
-  84=Fast Fusion,
-  85=Bebob,
-  86=Latin,
-  87=Revival,
-  88=Celtic,
-  89=Bluegrass,
-  90=Avantgarde,
-  91=Gothic Rock,
-  92=Progressive Rock,
-  93=Psychedelic Rock,
-  94=Symphonic Rock,
-  95=Slow Rock,
-  96=Big Band,
-  97=Chorus,
-  98=Easy Listening,
-  99=Acoustic,
- 100=Humour,
- 101=Speech,
- 102=Chanson,
- 103=Opera,
- 104=Chamber Music,
- 105=Sonata,
- 106=Symphony,
- 107=Booty Bass,
- 108=Primus,
- 109=Porn Groove,
- 110=Satire,
- 111=Slow Jam,
- 112=Club,
- 113=Tango,
- 114=Samba,
- 115=Folklore,
- 116=Ballad,
- 117=Power Ballad,
- 118=Rhythmic Soul,
- 119=Freestyle,
- 120=Duet,
- 121=Punk Rock,
- 122=Drum Solo,
- 123=Acapella,
- 124=Euro-House,
- 125=Dance Hall.


Appendix B - Author's Address
-----------------------------

Written by:

>**Martin Nilsson**  
>Rydsvägen 246 C. 30  
>S-584 34 Linköping  
>Sweden  
>Email: <nilsson@id3.org>

Edited by

>**Dirk Mahoney**  
>57 Pechey Street  
>Chermside Q  
>Australia 4032  
>Email: <dirk@id3.org>


>**Johan Sundström**  
>Alsättersgatan 5 A. 34  
>S-584 35 Linköping  
>Sweden  
>Email: <johan@id3.org>


id3v2.3.0 (last edited 2006-12-18 06:24:49 by DanONeill)  
Copyright © 1998-2009 by their respective owners


> Written with [StackEdit](https://stackedit.io/).
