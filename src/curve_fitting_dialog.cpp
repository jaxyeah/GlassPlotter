/*****************************************************************************
 **                                                                         **
 **  This file is part of GlassPlotter.                                     **
 **                                                                         **
 **  GlassPlotter is free software: you can redistribute it and/or modify   **
 **  it under the terms of the GNU General Public License as published by   **
 **  the Free Software Foundation, either version 3 of the License, or      **
 **  (at your option) any later version.                                    **
 **                                                                         **
 **  GlassPlotter is distributed in the hope that it will be useful,        **
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of         **
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          **
 **  GNU General Public License for more details.                           **
 **                                                                         **
 **  You should have received a copy of the GNU General Public License      **
 **  along with GlassPlotter.  If not, see <http://www.gnu.org/licenses/>.  **
 **                                                                         **
 *****************************************************************************
 **  Author  : Hiiragi                                                      **
 **  Contact : heterophyllus.work@gmail.com                                 **
 **  Website : https://github.com/heterophyllus/glassplotter                **
 **  Date    : 2020-5-25                                                    **
 *****************************************************************************/

#include "curve_fitting_dialog.h"
#include "ui_curve_fitting_dialog.h"

#include <QTableWidget>
#include <QComboBox>
#include <QMessageBox>
#include <QLineEdit>

#include "glass.h"
#include "glass_catalog.h"

#include "Eigen/Dense"

using namespace Eigen;

CurveFittingDialog::CurveFittingDialog(QList<GlassCatalog*> catalogList, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CurveFittingDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Curve Fitting");

    m_catalogList = catalogList;

    m_table = ui->tableWidget;
    m_table->clear();
    m_table->setColumnCount(3); //x,y,comment
    m_table->setHorizontalHeaderLabels(QStringList() << "X" << "Y" << "Comment");

    m_comboBoxOrder = ui->comboBox_Order;
    m_comboBoxOrder->clear();
    for (int i = 1; i <= m_maxFittingOrder;i++){
        m_comboBoxOrder->addItem(QString::number(i));
    }

    QObject::connect(ui->pushButton_AddRow,    SIGNAL(clicked()), this, SLOT(addRow()));
    QObject::connect(ui->pushButton_DeleteRow, SIGNAL(clicked()), this, SLOT(deleteSelectedRow()));

}

CurveFittingDialog::~CurveFittingDialog()
{
    m_catalogList.clear();

    m_table->clear();

    delete ui;
}

void CurveFittingDialog::addRow()
{
    //m_table->setColumnCount(3);

    int nRow = m_table->rowCount();
    m_table->insertRow(nRow);

    QTableWidgetItem* item;
    item = new QTableWidgetItem;
    item->setText("");
    m_table->setItem(nRow,0,item);

    item = new QTableWidgetItem;
    item->setText("");
    m_table->setItem(nRow,1,item);

    item = new QTableWidgetItem;
    item->setText("");
    m_table->setItem(nRow,2,item);

    m_table->update();

}

void CurveFittingDialog::deleteSelectedRow()
{
    int row = m_table->currentRow();

    m_table->removeRow(row);
    m_table->update();
}

bool CurveFittingDialog::getFittingResult(QList<double>& result)
{
    //https://en.wikipedia.org/wiki/Polynomial_regression

    int N = m_comboBoxOrder->currentIndex() + 1; //order
    int M = m_table->rowCount(); //samples

    // check
    if(M == 0){
        QMessageBox::warning(this,tr("File"), tr("No point has been added"));
        return false;
    }

    if(m_table->rowCount() == 0){
        QMessageBox::warning(this,tr("File"), tr("Invalid table row"));
        return false;
    }

    double tempval;
    for(int i = 0; i < m_table->rowCount(); i++){
        for(int j = 0; j < 2; j++)
        {
            if(!m_table->item(i,j)){
                try{
                    tempval = m_table->item(i,j)->text().toDouble();
                }  catch (...) {
                    QMessageBox::warning(this,tr("File"), tr("Invalid input"));
                    return false;
                }
            }
        }
    }

    // sampling points
    MatrixXd X = MatrixXd::Zero(M, N+1);
    VectorXd y(M);

    for(int i = 0; i < M; i++)
    {
        for(int j = 0; j < N+1; j++){
            X(i,j) = pow(m_table->item(i,0)->text().toDouble(), j);
        }
        y(i) = m_table->item(i,1)->text().toDouble();
    }

    MatrixXd A = X.transpose()*X;
    VectorXd b = (X.transpose())*y;
    VectorXd beta = A.bdcSvd(ComputeThinU | ComputeThinV).solve(b);
    //VectorXd beta = (X.transpose()*X).inverse()*(X.transpose())*y;

    // return fitting result
    result = {0.0, 0.0, 0.0, 0.0};
    for(int k = 0; k <= N; k++)
    {
        result[k] = beta(k);
    }

    return true;
}
