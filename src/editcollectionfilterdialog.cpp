// (c) 2006 Giovanni Venturi <giovanni@kde-it.org>
// See COPYING file for licensing information.

#include <qlayout.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <ktoolbarbutton.h>

#include "collectiondb.h"
#include "debug.h"
#include "editcollectionfilterdialog.h"
#include "metabundle.h"

EditCollectionFilterDialog::EditCollectionFilterDialog( QWidget* parent, bool metaBundleKeywords, const QString &text )
    : KDialogBase( Plain, i18n("Edit Filter"), User1|User2|Default|Ok|Cancel,
      Cancel, parent, "editfilter", /*modal*/true, /*separator*/false ),
      m_filterText(text)
{
    // Redefine "Default" button
    KGuiItem defaultButton( i18n("&Append Filter"), "add" );
    setButtonWhatsThis( Default, i18n( "<qt><p>Clicking here you can add the defined condition. The \"OK\" button is intended to "
                                        "close the dialog and apply the defined filter. With this button you can add more than "
                                        "one condition, so to create a more complex filtering condition.</p></qt>" ) );
    setButtonTip(Default, i18n( "Add this filter condition to the list" ) );
    setButtonGuiItem( Default, defaultButton );

    // define "User1" button
    KGuiItem user1Button( i18n("&Clear Filter"), "remove" );
    setButtonWhatsThis( User1, i18n( "<p>Clicking here you will clear the collection filter. If you intend just "
                                     "undo last appending just click on \"Remove last filter\" button.</p>" ) );
    setButtonTip(User1, i18n( "Clear the collection filter" ) );
    setButtonGuiItem( User1, user1Button );

    // define "User2" button
    KGuiItem user2Button( i18n("&Remove Last Filter"), "undo" );
    setButtonWhatsThis( User2, i18n( "<p>Clicking here you will remove the last appended collection filter, so to undo last "
                                     "appending filter. You cannot undo more than one action.</p>" ) );
    setButtonTip(User2, i18n( "Remove last appendend collection filter" ) );
    setButtonGuiItem( User2, user2Button );

    m_mainLay = new QVBoxLayout( plainPage() );
    m_mainLay->activate();

    // no filter rule available
    m_appended = false;

    // text explanation of this dialog
    QLabel *label1 = new QLabel( plainPage(), "label1" );
    label1->setText( i18n("<p>Edit the collection filter for finding tracks with specific attributes"
                             ", e.g. you can look for a track that has a length of 3 minutes.</p>") );
    m_mainLay->addWidget( label1 );
    m_mainLay->addItem( new QSpacerItem( 10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum ) );

    // let see to the users what will be the resulting filter string
    QHBoxLayout *filterLayout = new QHBoxLayout( plainPage() );

    QLabel *label2 = new QLabel( i18n("Resulting filter:"), plainPage(), "label2");
    filterLayout->addWidget( label2 );
    filterLayout->addItem( new QSpacerItem( 5, 10, QSizePolicy::Minimum, QSizePolicy::Minimum ) );

    m_filterRule = new QLineEdit( plainPage(), "filterRule" );
    m_filterRule->setEnabled( false );
    m_filterRule->setText( text );
    filterLayout->addWidget( m_filterRule );
    m_mainLay->addLayout( filterLayout );
    m_mainLay->addItem( new QSpacerItem( 10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum ) );

    // choosing keyword filtering
    QHBoxLayout *keywordLayout = new QHBoxLayout( plainPage() );
    QLabel *label3 = new QLabel( i18n("Choose keyword:"), plainPage(), "label3" );
    QWhatsThis::add( label3,
      i18n("you can translate the keyword as you will do for the combobox",
           "<p>Here you can choose to <i>type text</i> directly or to use "
           "some keywords to specify some attributes as the artist name "
           "and so on. The keyword selectable are divided by their specific value. "
           "Some keywords are numeric and others are alphanumeric. You do not need "
           "to know it directly. When a keyword is numeric than will be enabled "
           "the part related to the numeric specifics.</p><p>The alphanumeric "
           "keywords are the following: <b>album</b>, <b>artist</b>, <b>filename</b> "
           " (including path), <b>mountpoint</b> (i.e. /home/user1), <b>filetype</b> "
           " (you can specify: mp3, ogg, flac... it will match the file extensions), "
           "<b>genre</b>, <b>comment</b>, <b>composer</b>, <b>directory</b>, <b>lyrics</b>, "
           "<b>title</b> and <b>label</b>.</p>"
           "<p>The numeric keywords are: <i>bitrate</i>, <i>disc/discnumber</i> "
           "<b>length</b> (expressed in seconds), <b>playcount</b>, <b>rating</b> "
           "<b>samplerate</b>, <b>score</b>, <b>size/filesize</b>, (expressed in bytes, "
           "kbytes and megabytes as specified in the unit for the filesize keyword) "
           "<b>track</b> (that is the track number) and <b>year</b>.</p>") );
    keywordLayout->addWidget( label3 );
    keywordLayout->addItem( new QSpacerItem( 5, 10, QSizePolicy::Minimum, QSizePolicy::Minimum ) );
    m_comboKeyword = new QComboBox( plainPage(), "keywordComboBox");
    QToolTip::add( m_comboKeyword, i18n("Select a keyword for the filter") );
    label3->setBuddy( m_comboKeyword );

    m_comboKeyword->insertItem( i18n("Type Text") );
    m_vector.push_back("type text");
    if( metaBundleKeywords )
    {
        for( int i=0; i < MetaBundle::NUM_COLUMNS; ++i )
        {
            if( i == MetaBundle::Mood )
                continue;

            m_comboKeyword->insertItem( MetaBundle::prettyColumnName( i ) );
            m_vector.push_back( MetaBundle::exactColumnName( i ).lower() );
        }
    }
    else
    {
        m_comboKeyword->insertItem( i18n("Album") );
        m_vector.push_back( "album" );
        m_comboKeyword->insertItem( i18n("Artist") );
        m_vector.push_back( "artist" );
        m_comboKeyword->insertItem( i18n("Bitrate") );
        m_vector.push_back( "bitrate" );
        m_comboKeyword->insertItem( i18n("Comment") );
        m_vector.push_back( "comment" );
        m_comboKeyword->insertItem( i18n("Composer") );
        m_vector.push_back( "composer" );
        m_comboKeyword->insertItem( i18n("Directory") );
        m_vector.push_back( "directory" );
        m_comboKeyword->insertItem( i18n("Disc Number") );
        m_vector.push_back( "disc" );
        m_comboKeyword->insertItem( i18n("Filename") );
        m_vector.push_back( "filename" );
        m_comboKeyword->insertItem( i18n("Mount Point") );
        m_vector.push_back( "mountpoint" );
        m_comboKeyword->insertItem( i18n("Filetype") );
        m_vector.push_back( "filetype" );
        m_comboKeyword->insertItem( i18n("Genre") );
        m_vector.push_back( "genre" );
        m_comboKeyword->insertItem( i18n("Length") );
        m_vector.push_back( "length" );
        m_comboKeyword->insertItem( i18n("Label") );
        m_vector.push_back( "label" );
        m_comboKeyword->insertItem( i18n("Lyrics") );
        m_vector.push_back( "lyrics" );
        m_comboKeyword->insertItem( i18n("Play Count") );
        m_vector.push_back( "playcount" );
        m_comboKeyword->insertItem( i18n("Rating") );
        m_vector.push_back( "rating" );
        m_comboKeyword->insertItem( i18n("Sample Rate") );
        m_vector.push_back( "samplerate" );
        m_comboKeyword->insertItem( i18n("Score") );
        m_vector.push_back( "score" );
        m_comboKeyword->insertItem( i18n("File Size") );
        m_vector.push_back( "size" );
        m_comboKeyword->insertItem( i18n("Title") );
        m_vector.push_back( "title" );
        m_comboKeyword->insertItem( i18n("Track") );
        m_vector.push_back( "track" );
        m_comboKeyword->insertItem( i18n("Year") );
        m_vector.push_back( "year" );
    }

    // the "type text" text is selected in the comboKeyword
    m_selectedIndex = 0;

    keywordLayout->addWidget( m_comboKeyword );
    keywordLayout->addItem( new QSpacerItem( 5, 10, QSizePolicy::Minimum, QSizePolicy::Minimum ) );
    m_editKeyword = new KLineEdit( plainPage(), "editKeywordBox" );
    QWhatsThis::add( m_editKeyword, i18n("<p>Type here the keyword attribute or type text to look for into the collection.</p>") );
    keywordLayout->addWidget( m_editKeyword );
    m_mainLay->addLayout( keywordLayout );
    m_mainLay->addItem( new QSpacerItem( 10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum ) );

    connect(m_comboKeyword, SIGNAL(activated(int)), this, SLOT(selectedKeyword(int)));

    // group of options on numeric attribute keywords: a value <,>,= ... or a value between Min and Max
    m_groupBox = new QGroupBox( plainPage(), "groupBox" );
    m_groupBox->setTitle( i18n( "keyword attribute value..." ) );
    m_mainLay->addWidget( m_groupBox );
    m_mainLay->addItem( new QSpacerItem( 10, 10, QSizePolicy::Minimum, QSizePolicy::Minimum ) );

    QVBoxLayout *vertLayout = new QVBoxLayout( m_groupBox, 15, 5 );

    // choose other keyword parameters: smaller than, greater than, equal to...
    QHBoxLayout *paramLayout2 = new QHBoxLayout( vertLayout );
    m_keywordValueRadio = new QRadioButton( i18n("has to be"), m_groupBox, "option2");
    paramLayout2->addWidget( m_keywordValueRadio );
    paramLayout2->addItem( new QSpacerItem( 5, 10, QSizePolicy::Fixed, QSizePolicy::Minimum ) );

    m_comboCondition = new QComboBox( m_groupBox, "valuecondition");
    m_comboCondition->insertItem( i18n("smaller than...") );
    m_comboCondition->insertItem( i18n("greater than...") );
    m_comboCondition->insertItem( i18n("equal to...") );
    paramLayout2->addWidget( m_comboCondition );
    paramLayout2->addItem( new QSpacerItem( 5, 10, QSizePolicy::Fixed, QSizePolicy::Minimum ) );

    m_spinValue1 = new QSpinBox( m_groupBox, "keywordvalue1" );
    paramLayout2->addWidget( m_spinValue1 );
    paramLayout2->addItem( new QSpacerItem( 5, 10, QSizePolicy::Fixed, QSizePolicy::Minimum ) );

    m_spinValue2 = new QSpinBox( m_groupBox, "keywordvalue2" );
    paramLayout2->addWidget( m_spinValue2 );

    // choose other keyword parameters: included between two values
    QHBoxLayout *paramLayout1 = new QHBoxLayout( vertLayout );
    m_minMaxRadio = new QRadioButton( i18n("included between"), m_groupBox, "option1");
    paramLayout1->addWidget( m_minMaxRadio );

    m_spinMin1 = new QSpinBox( m_groupBox, "minimum1" );
    paramLayout1->addWidget( m_spinMin1 );
    paramLayout1->addItem( new QSpacerItem( 5, 10, QSizePolicy::Minimum, QSizePolicy::Minimum ) );

    m_spinMin2 = new QSpinBox( m_groupBox, "minimum2" );
    paramLayout1->addWidget( m_spinMin2 );
    paramLayout1->addItem( new QSpacerItem( 5, 10, QSizePolicy::Minimum, QSizePolicy::Minimum ) );

    connect(m_spinMin1, SIGNAL(valueChanged(int)), this, SLOT(minSpinChanged(int)));

    QLabel *label4 = new QLabel( i18n("and"), m_groupBox, "label4");
    paramLayout1->addWidget( label4 );
    paramLayout1->addItem( new QSpacerItem( 5, 10, QSizePolicy::Minimum, QSizePolicy::Minimum ) );

    m_spinMax1 = new QSpinBox( m_groupBox, "maximum1" );
    paramLayout1->addWidget( m_spinMax1 );
    paramLayout1->addItem( new QSpacerItem( 5, 10, QSizePolicy::Minimum, QSizePolicy::Minimum ) );

    m_spinMax2 = new QSpinBox( m_groupBox, "maximum2" );
    paramLayout1->addWidget( m_spinMax2 );

    connect(m_spinMax1, SIGNAL(valueChanged(int)), this, SLOT(maxSpinChanged(int)));

    QHBoxLayout *filesizeLayout = new QHBoxLayout( vertLayout );
    QLabel *filesizeLabel = new QLabel( i18n("Unit for file size:"), m_groupBox, "filesizeLabel");
    filesizeLayout->addWidget( filesizeLabel );
    filesizeLayout->addItem( new QSpacerItem( 5, 10, QSizePolicy::Minimum, QSizePolicy::Minimum ) );
    m_comboUnitSize = new QComboBox( m_groupBox, "comboKiloMega" );
    filesizeLabel->setBuddy( m_comboUnitSize );
    m_comboUnitSize->insertItem( i18n("B (1 Byte)") );
    m_comboUnitSize->insertItem( i18n("KB (1024 Bytes)") );
    m_comboUnitSize->insertItem( i18n("MB (1024 KB)") );
    filesizeLayout->addWidget( m_comboUnitSize );

    // type text selected
    textWanted();

    // check the "One Value Choosing" by default
    chooseOneValue();

    connect(m_keywordValueRadio, SIGNAL(clicked()), SLOT(chooseOneValue()));
    connect(m_minMaxRadio, SIGNAL(clicked()), SLOT(chooseMinMaxValue()));

    QHBoxLayout *otherOptionsLayout = new QHBoxLayout( plainPage() );
    otherOptionsLayout->setAlignment( AlignHCenter );
    m_mainLay->addLayout( otherOptionsLayout );

    // the groupbox to select the action filter
    m_groupBox2 = new QGroupBox( plainPage(), "groupBox2" );
    m_groupBox2->setTitle( i18n( "Choose an action filter" ) );
    otherOptionsLayout->addWidget( m_groupBox2 );

    QVBoxLayout* ratioLay = new QVBoxLayout( m_groupBox2, 15, 0 );

    m_checkALL = new QRadioButton( i18n("select all words"), m_groupBox2, "checkall" );
    ratioLay->addWidget( m_checkALL );

    m_checkAtLeastOne = new QRadioButton( i18n("select at least one word"), m_groupBox2, "checkor");
    ratioLay->addWidget( m_checkAtLeastOne );

    m_checkExactly = new QRadioButton( i18n("select exactly the words"), m_groupBox2, "checkexactly");
    ratioLay->addWidget( m_checkExactly );

    m_checkExclude = new QRadioButton( i18n("do not select the words"), m_groupBox2, "checkexclude");
    ratioLay->addWidget( m_checkExclude );

    m_actionCheck << m_checkALL;
    m_actionCheck << m_checkAtLeastOne;
    m_actionCheck << m_checkExactly;
    m_actionCheck << m_checkExclude;

    connect( m_checkALL, SIGNAL(clicked()), this, SLOT(slotCheckAll()) );
    connect( m_checkAtLeastOne, SIGNAL(clicked()), this, SLOT(slotCheckAtLeastOne()) );
    connect( m_checkExactly, SIGNAL(clicked()), this, SLOT(slotCheckExactly()) );
    connect( m_checkExclude, SIGNAL(clicked()), this, SLOT(slotCheckExclude()) );

    // check "select all words" as default
    slotCheckAll();

    // some vertical space
    otherOptionsLayout->addItem( new QSpacerItem( 50, 5, QSizePolicy::Minimum, QSizePolicy::Minimum ) );

    QVBoxLayout* verticalCondLay = new QVBoxLayout( otherOptionsLayout, 15, 0 );

    m_groupBox3 = new QGroupBox( plainPage(), "groupBox3" );
    m_groupBox3->setTitle( i18n( "Appending condition" ) );
    verticalCondLay->addWidget( m_groupBox3 );

    QVBoxLayout* ratioLay2 = new QVBoxLayout( m_groupBox3, 15, 0 );

    m_checkAND = new QRadioButton( i18n("AND logic condition", "AND"), m_groupBox3, "checkAND" );
    ratioLay2->addWidget( m_checkAND );

    m_checkOR = new QRadioButton( i18n("OR logic condition", "OR"), m_groupBox3, "checkOR" );
    ratioLay2->addWidget( m_checkOR );

    otherOptionsLayout->addItem( new QSpacerItem( 10, 10, QSizePolicy::Minimum, QSizePolicy::Minimum ) );

    m_prefixNOT = new QCheckBox( i18n("Add NOT condition prefix"), plainPage(), "prefixNOT" );
    verticalCondLay->addWidget( m_prefixNOT );
    m_prefixNOT->setEnabled( false );

    connect(m_prefixNOT, SIGNAL(clicked()), SLOT(assignPrefixNOT()));

    m_mainLay->addItem( new QSpacerItem( 10, 20, QSizePolicy::Minimum, QSizePolicy::Minimum ) );

    // you need to append at least one filter condition to specify if do
    // an "AND" or an "OR" with the next condition if the filter is empty
    if (m_filterText.isEmpty())
      m_groupBox3->setEnabled( false );

    connect( m_checkAND, SIGNAL(clicked()), SLOT(slotCheckAND()) );
    connect( m_checkOR, SIGNAL(clicked()), SLOT(slotCheckOR()) );

    // check "AND" condition as default
    slotCheckAND();

    // setup Min Max Value spin
    setMinMaxValueSpins();

    //resize( 500, 470 );
}

EditCollectionFilterDialog::~EditCollectionFilterDialog()
{
  delete m_checkOR;
  delete m_checkAND;
  delete m_groupBox3;
  delete m_checkExclude;
  delete m_checkExactly;
  delete m_checkAtLeastOne;
  delete m_checkALL;

  delete m_groupBox2;
  delete m_spinMax2;
  delete m_spinMax1;
  delete m_spinMin2;
  delete m_spinMin1;

  delete m_minMaxRadio;
  delete m_comboUnitSize;
  delete m_spinValue2;
  delete m_spinValue1;
  delete m_comboCondition;
  delete m_keywordValueRadio;

  delete m_groupBox;
  delete m_editKeyword;
  delete m_comboKeyword;
  delete m_prefixNOT;
  delete m_filterRule;
  delete m_mainLay;
}

QString EditCollectionFilterDialog::filter() const
{
  return m_filterRule->text();
}

void EditCollectionFilterDialog::exclusiveSelectOf( int which )
{
  int size = static_cast<int>( m_actionCheck.count() );

  for ( int i = 0; i < size; i++ )
    if ( i != which )
      m_actionCheck[i]->setChecked( false );
    else
      m_actionCheck[i]->setChecked( true );
}

QString EditCollectionFilterDialog::keywordConditionString(const QString& keyword) const
{
  // this member is called when there is a keyword that needs numeric attributes
  QString result, unit;

  if (m_selectedIndex == 18)
    switch (m_comboUnitSize->currentItem())
    {
      case 1:
        // kbytes
        unit = "k";
        break;
      case 2:
        // mbytes
        unit = "m";
        break;
    }

  if (m_keywordValueRadio->isChecked())
    // less than..., greater than..., equal to...
    switch(m_comboCondition->currentItem())
    {
      case 0:
        // less than...
        result = m_strPrefixNOT + keyword + ":<";
        if (keyword == "length")
          result += QString::number( m_spinValue1->value() * 60 + m_spinValue2->value() ) + unit;
        else
          result += m_spinValue1->text() + unit;
        break;
      case 1:
        // greater than...
        result = m_strPrefixNOT + keyword + ":>";
        if (keyword == "length")
          result += QString::number( m_spinValue1->value() * 60 + m_spinValue2->value() ) + unit;
        else
          result += m_spinValue1->text() + unit;
        break;
      case 2:
        // equal to...
        if (keyword == "length")
          result = m_strPrefixNOT + "length:" + QString::number( m_spinValue1->value() * 60 + m_spinValue2->value() ) + unit;
        else
        {
          if (m_strPrefixNOT.isEmpty())
            result = keyword + ":>" + QString::number(m_spinValue1->value() - 1) + unit +
              " " + keyword + ":<" + QString::number(m_spinValue1->value() + 1) + unit;
          else
            result = keyword + ":<" + QString::number(m_spinValue1->value()) + unit +
              " OR " + keyword + ":>" + QString::number(m_spinValue1->value()) + unit;
        }
        break;
    }
  else
    // between...
    if (keyword == "length")
    {
      if (m_strPrefixNOT.isEmpty())
        result = "length:>" + QString::number( m_spinMin1->value() * 60 + m_spinMin2->value() - 1) + unit
          + " length:<" + QString::number( m_spinMax1->value() * 60 + m_spinMax2->value() + 1) + unit;
      else
        result = "length:<" + QString::number( m_spinMin1->value() * 60 + m_spinMin2->value()) + unit
          + " OR length:>" + QString::number( m_spinMax1->value() * 60 + m_spinMax2->value()) + unit;
    }
    else
    {
      if (m_strPrefixNOT.isEmpty())
        result = keyword + ":>" + QString::number(m_spinMin1->value() - 1) + unit +
          " " + keyword + ":<" + QString::number(m_spinMax1->value() + 1) + unit;
      else
        result = keyword + ":<" + QString::number(m_spinMin1->value() - 1) + unit +
          " OR " + keyword + ":>" + QString::number(m_spinMax1->value() + 1) + unit;
    }

  return result;
}

void EditCollectionFilterDialog::setMinMaxValueSpins()
{
  // setting some spin box options and limit values
  m_spinValue1->setValue( 0 );
  m_spinValue1->setMinValue( 0 );
  m_spinValue1->setMaxValue( 100000000 );

  m_spinValue2->setValue( 0 );
  m_spinValue2->setMinValue( 0 );
  m_spinValue2->setMaxValue( 59 );
  m_spinValue2->hide();

  m_spinMin1->setValue( 0 );
  m_spinMin1->setMinValue( 0 );
  m_spinMin1->setMaxValue( 100000000 );

  m_spinMin2->setMinValue( 0 );
  m_spinMin2->setMaxValue( 59 );
  m_spinMin2->hide();

  m_spinMax1->setValue( 1 );
  m_spinMax1->setMinValue( 0 );
  m_spinMax1->setMaxValue( 100000000 );

  m_spinMax2->setMinValue( 0 );
  m_spinMax2->setMaxValue( 59 );
  m_spinMax2->hide();

  // fix tooltip
  QToolTip::add( m_spinValue1, "" );
  QToolTip::add( m_spinValue2, i18n("seconds") );

  QToolTip::add( m_spinMin1, "" );
  QToolTip::add( m_spinMin2, i18n("seconds") );

  QToolTip::add( m_spinMax1, "" );
  QToolTip::add( m_spinMax2, i18n("seconds") );
}

// SLOTS
void EditCollectionFilterDialog::selectedKeyword(int index) // SLOT
{
    debug() << "you selected index " << index << ": '" << m_comboKeyword->text(index) << "'" << endl;
    m_groupBox2->setEnabled( false );
    m_comboUnitSize->setEnabled( false );
    m_prefixNOT->setEnabled( true );

    setMinMaxValueSpins();

    const QString key = m_vector[index];
    if( index == 0 )
    {
        // type text
        m_groupBox2->setEnabled( true );
        m_prefixNOT->setEnabled( false );
        textWanted();
    }
    else if( key=="bitrate" )
    {
        // bitrate: set useful values for the spinboxes
        m_spinValue1->setValue( 128 );
        m_spinMin1->setValue( 192 );
        m_spinMax1->setValue( 384 );
        valueWanted();
    }
    else if( key=="samplerate" )
    {
        // samplerate: set useful values for the spinboxes
        m_spinValue1->setValue( 44100 );
        m_spinMin1->setValue( 8000 );
        m_spinMax1->setValue( 48000 );
        valueWanted();
    }
    else if( key=="length" )
    {
        // length: set useful values for the spinboxes
        m_spinValue2->show();
        m_spinValue1->setValue( 3 );
        m_spinValue2->setValue( 0 );
        QToolTip::add( m_spinValue1, i18n("Minutes") );
        m_spinMin2->show();
        m_spinMax2->show();
        m_spinMin1->setValue( 1 );
        m_spinMax1->setValue( 5 );
        QToolTip::add( m_spinMin1, i18n("Minutes") );
        QToolTip::add( m_spinMax1, i18n("Minutes") );

        valueWanted();
    }
    else if( key=="size" || key=="filesize" )
    {
        // size: set useful values for the spinboxes
        m_comboUnitSize->setEnabled( true );
        m_spinValue1->setValue( 1 );
        m_spinMin1->setValue( 1 );
        m_spinMax1->setValue( 3 );
        m_comboUnitSize->setCurrentItem( 2 );
        valueWanted();
    }
    else if( key=="year" )
    {
        // year: set useful values for the spinboxes
        m_spinValue1->setValue( 1900 );
        m_spinMin1->setValue( 1900 );
        m_spinMax1->setValue( QDate::currentDate().year() );
        valueWanted();
    }
    else if( key=="track" || key=="disc" || key=="discnumber" )
    {
        // track/disc: set useful values for the spinboxes
        m_spinValue1->setValue( 1 );
        m_spinMin1->setValue( 0 );
        m_spinMax1->setValue( 15 );
        valueWanted();
    }
    else if( key=="playcount"
          || key=="rating"
          || key=="score" )
          
    {
        valueWanted();
    }
    else if( key=="label" )
        textWanted( CollectionDB::instance()->labelList() );
    else if( key=="album" )
        textWanted( CollectionDB::instance()->albumList() );
    else if( key=="artist" )
        textWanted( CollectionDB::instance()->artistList() );
    else if( key=="composer" )
        textWanted( CollectionDB::instance()->composerList() );
    else if( key=="genre" )
        textWanted( CollectionDB::instance()->genreList() );
    else if( key=="type" || key=="filetype" )
    {
        QStringList types;
        types << "mp3" << "flac" << "ogg" << "aac" << "m4a" << "mp4" << "mp2" << "ac3"
            << "wav" << "asf" << "wma";
        textWanted( types );
    }
    else
        textWanted();

    // assign the correct value to the m_strPrefixNOT
    assignPrefixNOT();

    // assign the right index
    m_selectedIndex = index;
}

void EditCollectionFilterDialog::minSpinChanged(int value) // SLOT
{
  if (value >= m_spinMax1->value())
    m_spinMax1->setValue(value + 1);
}

void EditCollectionFilterDialog::maxSpinChanged(int value) // SLOT
{
  if (m_spinMin1->value() >= value)
    m_spinMin1->setValue(value - 1);
}

void EditCollectionFilterDialog::textWanted() // SLOT
{
    m_editKeyword->setEnabled( true );
    m_groupBox->setEnabled( false );

    m_editKeyword->completionObject()->clear();
}

void EditCollectionFilterDialog::textWanted( const QStringList &completion ) // SLOT
{
    m_editKeyword->setEnabled( true );
    m_groupBox->setEnabled( false );

    m_editKeyword->completionObject()->clear();
    m_editKeyword->completionObject()->insertItems( completion );
    m_editKeyword->completionObject()->setIgnoreCase( true );
    m_editKeyword->setCompletionMode( KGlobalSettings::CompletionPopup );
}

void EditCollectionFilterDialog::valueWanted() // SLOT
{
  m_editKeyword->setEnabled( false );
  m_groupBox->setEnabled( true );
}

void EditCollectionFilterDialog::chooseOneValue() // SLOT
{
  m_keywordValueRadio->setChecked( true );
  m_minMaxRadio->setChecked( false );
}

void EditCollectionFilterDialog::chooseMinMaxValue() // SLOT
{
  m_keywordValueRadio->setChecked( false );
  m_minMaxRadio->setChecked( true );
}

void EditCollectionFilterDialog::slotCheckAll() // SLOT
{
  exclusiveSelectOf( 0 );
}

void EditCollectionFilterDialog::slotCheckAtLeastOne() // SLOT
{
  exclusiveSelectOf( 1 );
}

void EditCollectionFilterDialog::slotCheckExactly() // SLOT
{
  exclusiveSelectOf( 2 );
}

void EditCollectionFilterDialog::slotCheckExclude() // SLOT
{
  exclusiveSelectOf( 3 );
}

void EditCollectionFilterDialog::slotCheckAND() // SLOT
{
  m_checkAND->setChecked( true );
  m_checkOR->setChecked( false );
}

void EditCollectionFilterDialog::slotCheckOR() // SLOT
{
  m_checkAND->setChecked( false );
  m_checkOR->setChecked( true );
}

void EditCollectionFilterDialog::assignPrefixNOT() // SLOT
{
  if (m_prefixNOT->isChecked())
    m_strPrefixNOT = "-";
  else
    m_strPrefixNOT = "";
}

void EditCollectionFilterDialog::slotDefault() // SLOT
{
  // now append the filter rule if not empty
  if (m_editKeyword->text().isEmpty() && (m_selectedIndex == 0))
  {
    KMessageBox::sorry( 0, i18n("<p>Sorry but the filter rule cannot be set. The text field is empty. "
      "Please type something into it and retry.</p>"), i18n("Empty Text Field"));
    m_editKeyword->setFocus();
    return;
  }
  if (!m_appended)
  {
    // it's the first rule
    m_appended = true;
    m_groupBox3->setEnabled( true );
  }

  m_previousFilterText = m_filterText;
  if (!m_filterText.isEmpty())
  {
    m_filterText += " ";
    if (m_checkOR->isChecked())
      m_filterText += "OR ";
  }
  QStringList list = QStringList::split( " ", m_editKeyword->text() );
  const QString key = m_vector[m_selectedIndex];
  if( m_selectedIndex == 0 )
  {
      // type text
      debug() << "selected text: '" << m_editKeyword->text() << "'" << endl;
      if (m_actionCheck[0]->isChecked())
      {
        // all words
        m_filterText += m_editKeyword->text();
      }
      else if (m_actionCheck[1]->isChecked())
      {
        // at least one word
        m_filterText += *(list.begin());
        for ( QStringList::Iterator it = ++list.begin(); it != list.end(); ++it )
          m_filterText += " OR " + *it;
      }
      else if (m_actionCheck[2]->isChecked())
      {
        // exactly the words
        m_filterText += "\"" + m_editKeyword->text() + "\"";
      }
      else if (m_actionCheck[3]->isChecked())
      {
        // exclude words
        for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
          m_filterText += " -" + *it;
      }
  }
  else if( key=="bitrate"
          || key=="disc" || key=="discnumber"
          || key=="length"
          || key=="playcount"
          || key=="rating"
          || key=="samplerate"
          || key=="score"
          || key=="filesize" || key=="size"
          || key=="track"
          || key=="year" )
  {
      m_filterText += keywordConditionString( m_vector[m_selectedIndex] );
  }
  else
  {
      m_filterText += m_vector[m_selectedIndex] + ":" +  m_editKeyword->text();
  }
  m_filterRule->setText( m_filterText );
  emit filterChanged( m_filterText );
}

void EditCollectionFilterDialog::slotUser1() // SLOT
{
  m_previousFilterText = m_filterText;
  m_filterText = "";
  m_filterRule->setText( m_filterText );
  emit filterChanged( m_filterText );

  // no filter appended cause all cleared
  m_appended = false;
  m_groupBox3->setEnabled( false );
}

void EditCollectionFilterDialog::slotUser2() // SLOT
{
  m_filterText = m_previousFilterText;
  if (m_filterText.isEmpty())
  {
    // no filter appended cause all cleared
    m_appended = false;
    m_groupBox3->setEnabled( false );
  }
  m_filterRule->setText( m_filterText );
  emit filterChanged( m_filterText );
}

void EditCollectionFilterDialog::slotOk() // SLOT
{
  // avoid confusing people that think to click OK to set a filter
  if (m_appended)
    accept();
}

#include "editcollectionfilterdialog.moc"
