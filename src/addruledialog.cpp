//  This file is part of Qt Bitcion Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2014 July IGHOR <julyighor@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  In addition, as a special exception, the copyright holders give
//  permission to link the code of portions of this program with the
//  OpenSSL library under certain conditions as described in each
//  individual source file, and distribute linked combinations including
//  the two.
//
//  You must obey the GNU General Public License in all respects for all
//  of the code used other than OpenSSL. If you modify file(s) with this
//  exception, you may extend this exception to your version of the
//  file(s), but you are not obligated to do so. If you do not wish to do
//  so, delete this exception statement from your version. If you delete
//  this exception statement from all source files in the program, then
//  also delete it here.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "addruledialog.h"
#include "ui_addruledialog.h"
#include "main.h"
#include <QFileDialog>
#include "julyspinboxfix.h"
#include "percentpicker.h"
#include "rulescriptparser.h"
#include "exchange.h"
#include <QMessageBox>
#include "scriptobject.h"

AddRuleDialog::AddRuleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddRuleDialog)
{
    ui->setupUi(this);
    ui->buttonSaveRule->setVisible(false);
    ui->scriptCodeGroupbox->setVisible(false);
    on_thanAmountPercent_toggled(false);
    on_thanPricePercent_toggled(false);
    on_variableBPercent_toggled(false);

    ui->groupBoxWhen->setTitle(julyTr("WHEN","When"));

    Q_FOREACH(QDoubleSpinBox* spinBox, mainWindow.findChildren<QDoubleSpinBox*>())
    {
        QString scriptName=spinBox->whatsThis();
        if(scriptName.isEmpty())continue;
        QString translatedName=julyTranslator.translateString("INDICATOR_"+(scriptName.startsWith("Balance")?"BALANCE":scriptName.toUpper()),scriptName);
        if(scriptName.startsWith("BalanceA"))translatedName=translatedName.arg(baseValues.currentPair.currAStr); else
        if(scriptName.startsWith("BalanceB"))translatedName=translatedName.arg(baseValues.currentPair.currBStr);
        ui->variableA->insertItem(ui->variableA->count(),translatedName,scriptName);
        ui->variableB->insertItem(ui->variableB->count(),translatedName,scriptName);

        if(spinBox->accessibleName()=="PRICE")
            ui->thanPriceType->insertItem(ui->thanPriceType->count(),translatedName,scriptName);
    }
    ui->variableA->insertItem(ui->variableA->count(),julyTr("RULE_IMMEDIATELY_EXECUTION","Execute Immediately"),"IMMEDIATELY");
    ui->variableB->insertItem(ui->variableB->count(),julyTr("RULE_EXACT_VALUE","Exact value"),"EXACT");
    ui->thanPriceType->insertItem(ui->thanPriceType->count(),julyTr("RULE_EXACT_VALUE","Exact value"),"EXACT");

    int lastPriceInt=ui->variableA->findData("LastPrice");
    if(lastPriceInt>-1)ui->variableA->setCurrentIndex(lastPriceInt);
    lastPriceInt=ui->variableB->findData("LastPrice");
    if(lastPriceInt>-1)ui->variableB->setCurrentIndex(lastPriceInt);
    lastPriceInt=ui->thanPriceType->findData("LastPrice");
    if(lastPriceInt>-1)ui->thanPriceType->setCurrentIndex(lastPriceInt);

    ui->thanType->insertItem(ui->thanType->count(),julyTr("RULE_THAN_SELL","Sell %1").arg(baseValues.currentPair.currAStr),"TRADE");
    ui->thanType->insertItem(ui->thanType->count(),julyTr("RULE_THAN_BUY","Buy %1").arg(baseValues.currentPair.currAStr),"TRADE");

    ui->thanType->insertItem(ui->thanType->count(),julyTr("RULE_THAN_RECEIVE","Receive %1").arg(baseValues.currentPair.currBStr),"TRADE");
    ui->thanType->insertItem(ui->thanType->count(),julyTr("RULE_THAN_SPEND","Spend %1").arg(baseValues.currentPair.currBStr),"TRADE");

    ui->thanType->insertItem(ui->thanType->count(),julyTr("RULE_THAN_CANCEL_ALL","Cancel All Orders"),"NOPARAMS");
    ui->thanType->insertItem(ui->thanType->count(),julyTr("RULE_THAN_CANCEL_ASKS","Cancel Asks"),"NOPARAMS");
    ui->thanType->insertItem(ui->thanType->count(),julyTr("RULE_THAN_CANCEL_BIDS","Cancel Bids"),"NOPARAMS");
    ui->thanType->insertItem(ui->thanType->count(),julyTr("RULE_THAN_START_GROUP","Start Group or Script"),"NAME");
    ui->thanType->insertItem(ui->thanType->count(),julyTr("RULE_THAN_STOP_GROUP","Stop Group or Script"),"NAME");
    ui->thanType->insertItem(ui->thanType->count(),julyTr("RULE_THAN_BEEP","Beep"),"BEEP");
    ui->thanType->insertItem(ui->thanType->count(),julyTr("RULE_THAN_PLAY","Play Sound"),"PLAY");
    ui->thanType->insertItem(ui->thanType->count(),julyTr("RULE_THAN_START_APP","Start Application"),"PROGRAM");
    //ui->thanType->insertItem(ui->thanType->count(),julyTr("RULE_THAN_SAY_TEXT","Say Text"),"SAY");

    ui->variableBMode->setItemText(0,julyTr("RULE_TYPE_REALTIME","Realtime comparation"));
    ui->variableBMode->setItemText(1,julyTr("RULE_TYPE_SAVEONSTART","Fixed. Save base value once at rule starts"));
    ui->variableBMode->setItemText(2,julyTr("RULE_TYPE_FIXED","Trailing. Save base value on opposide direction"));

    ui->variableBFee->setItemText(0,julyTr("RULE_NOFEE","No Fee"));
    ui->variableBFee->setItemText(1,julyTr("RULE_PLUSFEE","+ Fee"));
    ui->variableBFee->setItemText(2,julyTr("RULE_MINUSFEE","- Fee"));

    ui->thanAmountFee->setItemText(0,julyTr("RULE_NOFEE","No Fee"));
    ui->thanAmountFee->setItemText(1,julyTr("RULE_PLUSFEE","+ Fee"));
    ui->thanAmountFee->setItemText(2,julyTr("RULE_MINUSFEE","- Fee"));

    ui->thanPriceFee->setItemText(0,julyTr("RULE_NOFEE","No Fee"));
    ui->thanPriceFee->setItemText(1,julyTr("RULE_PLUSFEE","+ Fee"));
    ui->thanPriceFee->setItemText(2,julyTr("RULE_MINUSFEE","- Fee"));

    on_thanType_currentIndexChanged(ui->thanType->currentIndex());
    on_variableA_currentIndexChanged(ui->variableA->currentIndex());
    on_variableB_currentIndexChanged(ui->variableB->currentIndex());
    on_valueBSymbol_currentIndexChanged(ui->valueBSymbol->currentIndex());
    on_thanSymbol_currentIndexChanged(ui->thanSymbol->currentIndex());
    on_valueASymbol_currentIndexChanged(ui->valueASymbol->currentIndex());

    Q_FOREACH(QDoubleSpinBox* spinBox, findChildren<QDoubleSpinBox*>())new JulySpinBoxFix(spinBox);

    QString baseSymbol=baseValues.currentPair.symbol;
    Q_FOREACH(QComboBox *comboBox, findChildren<QComboBox*>())
    {
        if(comboBox->accessibleName()!="SYMBOL")continue;

        int selectedRow=-1;
        for(int n=0;n<mainWindow.currPairsList.count();n++)
        {
            QString curSymbol=mainWindow.currPairsList.at(n).symbol;
            if(curSymbol==baseSymbol)selectedRow=n;
            //else if(!currentExchange->multiCurrencyTradeSupport)continue;
            comboBox->insertItem(comboBox->count(),mainWindow.currPairsList.at(n).name,curSymbol);
        }
        if(selectedRow>-1)comboBox->setCurrentIndex(selectedRow);
    }

    Q_FOREACH(QComboBox *comboBox, findChildren<QComboBox*>())connect(comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(reCacheCode()));
    Q_FOREACH(QDoubleSpinBox *spinBox, findChildren<QDoubleSpinBox*>())connect(spinBox,SIGNAL(valueChanged(double)),this,SLOT(reCacheCode()));
    Q_FOREACH(QCheckBox *checkBox, findChildren<QCheckBox*>())connect(checkBox,SIGNAL(toggled(bool)),this,SLOT(reCacheCode()));
    Q_FOREACH(QRadioButton *checkBox, findChildren<QRadioButton*>())connect(checkBox,SIGNAL(toggled(bool)),this,SLOT(reCacheCode()));


    setWindowTitle("Qt Bitcoin Trader ["+parent->windowTitle()+"]");
    setWindowFlags(Qt::WindowCloseButtonHint);

    setWindowTitle(julyTranslator.translateButton("ADD_RULES","Add Rule"));

    julyTranslator.translateUi(this);
    mainWindow.fixAllChildButtonsAndLabels(this);

    setMinimumWidth(mainWindow.minimumSizeHint().width());
    setMinimumHeight(300);
    fixSize();
    QTimer::singleShot(200,this,SLOT(reCacheCode()));
    QTimer::singleShot(201,this,SLOT(fixSize()));
}

AddRuleDialog::~AddRuleDialog()
{
    delete ui;
}

void AddRuleDialog::fillByHolder(RuleHolder &holder)
{
    ui->thanAmountPercent->setChecked(holder.thanAmountPercentChecked);
    ui->thanPricePercent->setChecked(holder.thanPricePercentChecked);
    ui->variableBPercent->setChecked(holder.variableBPercentChecked);

    setComboIndex(ui->thanAmountFee,holder.thanAmountFeeIndex);
    setComboIndex(ui->thanPriceFee,holder.thanPriceFeeIndex);
    setComboIndex(ui->thanType, holder.thanTypeIndex);
    setComboIndex(ui->variableBFee,holder.variableBFeeIndex);
    setComboIndex(ui->variableBMode,holder.variableBModeIndex);

    ui->thanAmount->setValue(holder.thanAmount);
    ui->thanPriceValue->setValue(holder.thanPrice);
    ui->variableBExact->setValue(holder.variableBExact);

    ui->comparation->setCurrentIndex(ui->comparation->findText(holder.comparationText));

    setComboIndex(ui->thanPricePlusMinus,holder.thanPricePlusMinusText);
    setComboIndex(ui->variableBplusMinus,holder.variableBplusMinus);

    setComboIndexByData(ui->thanPriceType,holder.thanPriceTypeCode);

    ui->thanText->setText(holder.thanText);

    setComboIndexByData(ui->thanSymbol,holder.tradeSymbolCode);
    setComboIndexByData(ui->valueASymbol,holder.valueASymbolCode);
    setComboIndexByData(ui->valueBSymbol,holder.valueBSymbolCode);
    setComboIndexByData(ui->variableA,holder.variableACode);
    setComboIndexByData(ui->variableB,holder.variableBCode);
    setComboIndexByData(ui->valueBSymbol,holder.variableBSymbolCode);

    ui->descriptionText->setText(holder.description);

    ui->delayValue->setValue(holder.delaySeconds);

    ui->buttonSaveRule->setVisible(true);
    ui->buttonAddRule->setVisible(false);
}

bool AddRuleDialog::isRuleEnabled()
{
    return ui->ruleIsEnabled->isChecked();
}

void AddRuleDialog::setComboIndexByData(QComboBox *list, QString &data)
{
    if(list==0)return;
    int find=list->findData(data);
    if(find<0)return;
    list->setCurrentIndex(find);
}

void AddRuleDialog::setComboIndex(QComboBox *list, QString &text)
{
    if(list==0)return;
    int find=list->findData(text);
    if(find<0)return;
    list->setCurrentIndex(find);
}

void AddRuleDialog::setComboIndex(QComboBox *list, int &row)
{
    if(list==0)return;
    if(row<0||row>=list->count())return;
    list->setCurrentIndex(row);
}

RuleHolder AddRuleDialog::getRuleHolder()
{
    RuleHolder holder;
    holder.thanAmountPercentChecked=ui->thanAmountPercent->isChecked();
    holder.thanPricePercentChecked=ui->thanPricePercent->isChecked();
    holder.variableBPercentChecked=ui->variableBPercent->isVisible()&&ui->variableBPercent->isChecked();
    holder.thanAmountFeeIndex=ui->thanAmountFee->currentIndex();
    holder.thanPriceFeeIndex=ui->thanPriceFee->currentIndex();
    holder.thanTypeIndex=ui->thanType->currentIndex();
    holder.variableBFeeIndex=!ui->variableBFee->isVisible()?-1:ui->variableBFee->currentIndex();
    holder.variableBModeIndex=ui->variableBMode->currentIndex();
    holder.thanAmount=ui->thanAmount->value();
    holder.thanPrice=ui->thanPriceValue->value();
    holder.variableBExact=ui->variableBExact->value();
    holder.comparationText=ui->comparation->currentText();
    holder.thanPricePlusMinusText=ui->thanPricePlusMinus->currentText();
    holder.thanPriceTypeCode=comboCurrentData(ui->thanPriceType);
    holder.thanText=ui->thanText->text();
    holder.tradeSymbolCode=comboCurrentData(ui->thanSymbol);
    holder.valueASymbolCode=comboCurrentData(ui->valueASymbol);
    holder.valueBSymbolCode=comboCurrentData(ui->valueBSymbol);
    holder.variableACode=comboCurrentData(ui->variableA);
    holder.variableBCode=comboCurrentData(ui->variableB);
    holder.variableBplusMinus=ui->variableBplusMinus->currentText();
    holder.variableBSymbolCode=comboCurrentData(ui->valueBSymbol);
    holder.description=ui->descriptionText->text();
    holder.delaySeconds=ui->delayValue->value();
    return holder;
}

void AddRuleDialog::reCacheCode()
{
    QString descriptionText;
    if(ui->delayValue->value()>0)
    {
        descriptionText=julyTr("DELAY_SEC","Delay %1 sec").arg(mainWindow.numFromDouble(ui->delayValue->value()))+" ";
    }
    if(ui->variableA->currentIndex()==ui->variableA->count()-1)
    {
        if(descriptionText.isEmpty())descriptionText=julyTr("RULE_IMMEDIATELY_EXECUTION","Execute immediately");
        else descriptionText.remove(descriptionText.size()-1,1);
    }
    else
    {
        descriptionText+=julyTr("WHEN","When")+" "+ui->variableA->currentText();
        if(ui->valueASymbol->isVisible())descriptionText+=" ("+ui->valueASymbol->currentText()+")";
        descriptionText+=" "+ui->comparation->currentText()+" "+ui->variableB->currentText();
        if(ui->valueBSymbol->isVisible())descriptionText+=" ("+ui->valueBSymbol->currentText()+")";

        if(ui->variableBExact->value()!=0.0)
        {
            if(comboCurrentData(ui->variableB)!="EXACT")descriptionText+=" "+ui->variableBplusMinus->currentText();
            descriptionText+=" "+mainWindow.numFromDouble(ui->variableBExact->value());
            if(ui->variableBPercent->isChecked())descriptionText+="%";
        }
        if(ui->variableBFee->currentIndex()>0)
        {
            if(ui->variableBFee->currentIndex()==1)descriptionText+=" + ";
            else descriptionText+=" - ";
            descriptionText+=julyTr("LOG_FEE","fee");
        }
        if(ui->variableBMode->isVisible())descriptionText+=" ("+ui->variableBMode->currentText()+")";
    }
    descriptionText+=" "+julyTr("THEN","then")+" "+ui->thanType->currentText();
    if(ui->thanSymbol->isVisible())descriptionText=descriptionText+" ("+ui->thanSymbol->currentText()+")";

    if(ui->thanAmount->isVisible())
    {
        QString sign;
        CurrencyPairItem pairItem;
        pairItem=baseValues.currencyPairMap.value(comboCurrentData(ui->valueBSymbol),pairItem);
        if(!pairItem.symbol.isEmpty())sign=pairItem.currASign;

        descriptionText+=" "+sign+mainWindow.numFromDouble(ui->thanAmount->value());
        if(ui->thanAmountPercent->isChecked())descriptionText+="%";

        if(ui->thanAmountFee->currentIndex()>0)
        {
            if(ui->thanAmountFee->currentIndex()==1)descriptionText+=" + ";
            else descriptionText+=" - ";
            descriptionText+=julyTr("LOG_FEE","fee");
        }
    }
    if(ui->thanPriceType->isVisible())
    {
        QString atPrice;
        bool priceExact=comboCurrentData(ui->thanPriceType)==QLatin1String("EXACT");
        if(!priceExact)atPrice=ui->thanPriceType->currentText();

        if(ui->thanPriceValue->isVisible()&&ui->thanPriceValue->value()!=0.0)
        {
            if(!priceExact)atPrice+=" "+ui->thanPricePlusMinus->currentText();

            QString sign;
            CurrencyPairItem pairItem;
            pairItem=baseValues.currencyPairMap.value(comboCurrentData(ui->thanSymbol),pairItem);
            if(!pairItem.symbol.isEmpty())sign=pairItem.currBSign;

            atPrice+=" "+sign+mainWindow.numFromDouble(ui->thanPriceValue->value());
        }

        if(ui->thanPriceFee->currentIndex()>0)
        {
            if(ui->thanPriceFee->currentIndex()==1)atPrice+=" + ";
            else atPrice+=" - ";
            atPrice+=julyTr("LOG_FEE","fee");
        }

        descriptionText+=" "+julyTr("AT","at %1").arg(atPrice);
    }
    if(ui->thanText->isVisible())descriptionText+=" ("+ui->thanText->text()+")";

    ui->descriptionText->setText(descriptionText);

    RuleHolder holder=getRuleHolder();
    ui->scriptCodePreview->setPlainText(RuleScriptParser::holderToScript(holder));
}

void AddRuleDialog::fixSize()
{
    QSize preferedSize=minimumSizeHint();
    if(ui->scriptCodeGroupbox->isVisible())
        resize(qMax(preferedSize.width(),800),height());
    else
    resize(qMax(preferedSize.width(),800),qMax(preferedSize.height(),250));
}

void AddRuleDialog::on_variableA_currentIndexChanged(int index)
{
    bool immediately=index==ui->variableA->count()-1;
    ui->whenValueGroupBox->setEnabled(!immediately);
    ui->comparation->setEnabled(!immediately);
    ui->valueALabel->setVisible(!immediately);
    ui->valueASymbol->setVisible(!immediately);
    fixSize();
}

void AddRuleDialog::on_variableB_currentIndexChanged(int index)
{
    QString varBType=comboData(ui->variableB,index);
    bool exact=varBType=="EXACT";
    ui->variableBplusMinus->setVisible(!exact);
    ui->variableBFee->setVisible(!exact);
    ui->modeFon->setVisible(!exact);
    ui->variableBModeLabel->setVisible(!exact);
    ui->variableBPercent->setVisible(!exact);
    on_variableBPercent_toggled(ui->variableBPercent->isChecked());

    fixSize();
}

void AddRuleDialog::on_thanAmountPercent_toggled(bool checked)
{
    ui->thanAmountPercentButton->setVisible(checked);
    if(ui->thanAmountPercentButton->isVisible())
    {
        ui->thanAmount->setMaximum(200.0);
        ui->thanAmount->setMinimum(0.00000001);
    }
    else
    {
        ui->thanAmount->setMaximum(9999999999.9);
        ui->thanAmount->setMinimum(-9999999999.9);
    }
    fixSize();
}

void AddRuleDialog::on_thanType_currentIndexChanged(int index)
{
    currentThanType=comboData(ui->thanType,index);
    bool trade=currentThanType=="TRADE";
    bool noParams=currentThanType=="NOPARAM";
    bool name=currentThanType=="NAME";
    bool beep=currentThanType=="BEEP";
    bool wav=currentThanType=="PLAY";
    bool say=false;//currentThanType=="SAY";
    bool program=currentThanType=="PROGRAM";

    ui->doubleValueFon->setVisible(!noParams&&trade);
    ui->thanTextBrowse->setVisible(!noParams&&(wav||program));
    ui->thanText->setVisible(!noParams&&(wav||program||name||say));
    ui->clearTextButton->setVisible(ui->thanText->isVisible());
    ui->nameLabel->setVisible(name);
    ui->thanSymbol->setVisible(trade);
    ui->thanSymbolLabel->setVisible(trade);

    ui->playButton->setVisible(!noParams&&(wav||beep||say));

    fixSize();

    if(wav||program)on_thanTextBrowse_clicked();
}

void AddRuleDialog::on_playButton_clicked()
{
    //if(currentThanType=="SAY")mainWindow.sayText(thanText);
    //else
    if(currentThanType=="BEEP")mainWindow.beep();
    else
    if(currentThanType=="PLAY")mainWindow.playWav(ui->thanText->text());
}

void AddRuleDialog::on_thanTextBrowse_clicked()
{
    QString lastRulesDir=mainWindow.iniSettings->value("UI/LastRulesPath",baseValues.desktopLocation).toString();
    if(!QFile::exists(lastRulesDir))lastRulesDir=baseValues.desktopLocation;

    QString description;
    QString mask="*";

    if(currentThanType=="PLAY"){description=julyTr("PLAY_SOUND_WAV","Open WAV file");mask="*.wav";}
    if(currentThanType=="PROGRAM"){description=julyTr("OPEN_ANY_FILE","Open any file");mask="*";}

    QString fileName=QFileDialog::getOpenFileName(this, description, lastRulesDir,"("+mask+")");
    if(fileName.isEmpty())return;
    mainWindow.iniSettings->setValue("UI/LastRulesPath",QFileInfo(fileName).dir().path());
    mainWindow.iniSettings->sync();

    #ifdef Q_OS_WIN
        fileName.replace("/","\\");
    #endif

    ui->thanText->setText(fileName);
}

void AddRuleDialog::on_thanPriceType_currentIndexChanged(int index)
{
    QString thanPriceType=comboData(ui->thanPriceType,index);
    bool exactPrice=thanPriceType=="EXACT";

    ui->thanPricePlusMinus->setVisible(!exactPrice);
    ui->thanPricePercent->setVisible(!exactPrice);
    if(ui->thanPricePercentButton->isVisible())
    {
        ui->thanPriceValue->setMaximum(200.0);
        ui->thanPriceValue->setMinimum(0.00000001);
    }
    else
    {
        ui->thanPriceValue->setMaximum(9999999999.9);
        ui->thanPriceValue->setMinimum(-9999999999.9);
    }
    ui->thanPriceFee->setVisible(!exactPrice);

    on_thanPricePercent_toggled(ui->thanPricePercent->isChecked());
    fixSize();
}

void AddRuleDialog::on_variableBPercent_toggled(bool checked)
{
    ui->variableBPercentButton->setVisible(checked&&ui->variableBPercent->isVisible());
    if(ui->variableBPercentButton->isVisible())
    {
        ui->variableBExact->setMaximum(200.0);
        ui->variableBExact->setMinimum(0.00000001);
    }
    else
    {
        ui->variableBExact->setMaximum(9999999999.9);
        ui->variableBExact->setMinimum(-9999999999.9);
    }
    fixSize();
}

void AddRuleDialog::on_thanPricePercent_toggled(bool checked)
{
    ui->thanPricePercentButton->setVisible(checked);
    if(ui->thanPricePercentButton->isVisible())
    {
        ui->thanPriceValue->setMaximum(200.0);
        ui->thanPriceValue->setMinimum(0.00000001);
    }
    else
    {
        ui->thanPriceValue->setMaximum(9999999999.9);
        ui->thanPriceValue->setMinimum(-9999999999.9);
    }
    fixSize();
}

void AddRuleDialog::on_variableBPercentButton_clicked()
{
    PercentPicker *percentPicker=new PercentPicker(ui->variableBExact,100.0);
    QPoint execPos=ui->whenValueGroupBox->mapToGlobal(ui->variableBPercentButton->geometry().center());
    execPos.setX(execPos.x()-percentPicker->width()/2);
    execPos.setY(execPos.y()-percentPicker->width());
    percentPicker->exec(execPos);
}

void AddRuleDialog::on_thanAmountPercentButton_clicked()
{
    PercentPicker *percentPicker=new PercentPicker(ui->thanAmount,100.0);
    QPoint execPos=ui->actionGroupBox->mapToGlobal(ui->thanAmountPercentButton->geometry().center());
    execPos.setX(execPos.x()-percentPicker->width()/2);
    execPos.setY(execPos.y()-percentPicker->width());
    percentPicker->exec(execPos);
}

void AddRuleDialog::on_thanPricePercentButton_clicked()
{
    PercentPicker *percentPicker=new PercentPicker(ui->thanPriceValue,100.0);
    QPoint execPos=ui->actionGroupBox->mapToGlobal(ui->thanPricePercentButton->geometry().center());
    execPos.setX(execPos.x()-percentPicker->width()/2);
    execPos.setY(execPos.y()-percentPicker->width());
    percentPicker->exec(execPos);
}

void AddRuleDialog::on_variableBFee_currentIndexChanged(int)
{
    fixSize();
}

void AddRuleDialog::on_thanAmountFee_currentIndexChanged(int)
{
    fixSize();
}

void AddRuleDialog::on_thanPriceFee_currentIndexChanged(int)
{
    fixSize();
}

void AddRuleDialog::on_thanText_textChanged(const QString &)
{
    reCacheCode();
}

void AddRuleDialog::on_codePreview_toggled(bool checked)
{
    if(checked)
    {
        ui->addRuleGroupBox->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Maximum);
        setMaximumHeight(1280);
        resize(width(),640);
        fixSize();
    }
    else
    {
        ui->addRuleGroupBox->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
        setMaximumHeight(600);
        fixSize();
    }
}

QString AddRuleDialog::comboData(QComboBox *list, int row)
{
    if(row<0||row>=list->count())return QLatin1String("");
    return list->itemData(row).toString();
}

QString AddRuleDialog::comboCurrentData(QComboBox *list)
{
    return comboData(list,list->currentIndex());
}

void AddRuleDialog::on_buttonAddRule_clicked()
{
    RuleHolder holder=getRuleHolder();

    if(!holder.isValid())
    {
        QMessageBox::warning(this,windowTitle(),julyTr("INVALID_RULE_VALUES","Values you entered is invalid"));
        return;
    }

    if(ui->ruleIsEnabled->isChecked())
    {
        QString code=RuleScriptParser::holderToScript(holder,true);
        ScriptObject tempScript("Test");
        tempScript.executeScript(code,true);
        tempScript.stopScript();

        if(tempScript.testResult==1)
        {
        QMessageBox::warning(this,windowTitle(),julyTr("INVALID_RULE_CHECK","This rule will be executed instantly.<br>This means that you make a mistake.<br>Please check values you entered."));
        return;
        }
    }
    if(!baseValues.currentExchange_->multiCurrencyTradeSupport)
    {
        QString currentSymbol=baseValues.currentPair.symbol;
        if(comboCurrentData(ui->valueASymbol)!=currentSymbol||comboCurrentData(ui->valueBSymbol)!=currentSymbol||comboCurrentData(ui->thanSymbol)!=currentSymbol)
        {
            QMessageBox::warning(this,windowTitle(),julyTr("RULE_MULTITRADE_NOTSUPPORTED","Warning. Multi currency trading is not supported yet.\nRule will works only with the same currency pair as current.\nSet up all symbols as current main currency."));
            return;
        }
    }
        accept();
}

void AddRuleDialog::on_buttonSaveRule_clicked()
{
    on_buttonAddRule_clicked();
}

void AddRuleDialog::on_fillFromBuyPanel_clicked()
{
    setComboIndexByData(ui->thanSymbol,baseValues.currentPair.symbol);
    ui->thanPricePercent->setChecked(false);
    ui->thanPriceFee->setCurrentIndex(0);
    ui->thanType->setCurrentIndex(1);
    ui->thanPriceType->setCurrentIndex(ui->thanPriceType->count()-1);
    ui->thanAmount->setValue(mainWindow.ui.buyTotalBtc->value());
    ui->thanPriceValue->setValue(mainWindow.ui.buyPricePerCoin->value());
}

void AddRuleDialog::on_fillFromSellPanel_clicked()
{
    setComboIndexByData(ui->thanSymbol,baseValues.currentPair.symbol);
    ui->thanPricePercent->setChecked(false);
    ui->thanPriceFee->setCurrentIndex(0);
    ui->thanType->setCurrentIndex(0);
    ui->thanPriceType->setCurrentIndex(ui->thanPriceType->count()-1);
    ui->thanAmount->setValue(mainWindow.ui.sellTotalBtc->value());
    ui->thanPriceValue->setValue(mainWindow.ui.sellPricePerCoin->value());
}

void AddRuleDialog::on_valueBSymbol_currentIndexChanged(int index)
{
    QString symbol=ui->valueBSymbol->itemData(index).toString();

    CurrencyPairItem pairItem;
    pairItem=baseValues.currencyPairMap.value(symbol,pairItem);
    if(pairItem.symbol.isEmpty())return;

    for(int n=0;n<ui->variableB->count();n++)
    {
    QString scriptName=ui->variableB->itemData(n).toString();
    if(!scriptName.startsWith("Balance"))continue;

    QString translatedName=julyTranslator.translateString("INDICATOR_BALANCE",scriptName);
    if(scriptName.startsWith("BalanceA"))translatedName=translatedName.arg(pairItem.currAStr); else
    if(scriptName.startsWith("BalanceB"))translatedName=translatedName.arg(pairItem.currBStr);
    ui->variableB->setItemText(n,translatedName);
    }
}

void AddRuleDialog::on_thanSymbol_currentIndexChanged(int index)
{
    QString symbol=ui->thanSymbol->itemData(index).toString();

    CurrencyPairItem pairItem;
    pairItem=baseValues.currencyPairMap.value(symbol,pairItem);
    if(pairItem.symbol.isEmpty())return;

    ui->thanType->setItemText(0,julyTr("RULE_THAN_SELL","Sell %1").arg(pairItem.currAStr));
    ui->thanType->setItemText(1,julyTr("RULE_THAN_BUY","Buy %1").arg(pairItem.currAStr));
    ui->thanType->setItemText(2,julyTr("RULE_THAN_RECEIVE","Receive %1").arg(pairItem.currBStr));
    ui->thanType->setItemText(3,julyTr("RULE_THAN_SPEND","Spend %1").arg(pairItem.currBStr));
}

void AddRuleDialog::on_valueASymbol_currentIndexChanged(int index)
{
    QString symbol=comboData(ui->valueASymbol,index);

    CurrencyPairItem pairItem;
    pairItem=baseValues.currencyPairMap.value(symbol,pairItem);
    if(pairItem.symbol.isEmpty())return;

    for(int n=0;n<ui->variableA->count();n++)
    {
    QString scriptName=comboData(ui->variableA,n);
    if(!scriptName.startsWith("Balance"))continue;

    QString translatedName=julyTranslator.translateString("INDICATOR_BALANCE",scriptName);
    if(scriptName.startsWith("BalanceA"))translatedName=translatedName.arg(pairItem.currAStr); else
    if(scriptName.startsWith("BalanceB"))translatedName=translatedName.arg(pairItem.currBStr);
    ui->variableA->setItemText(n,translatedName);
    }
}