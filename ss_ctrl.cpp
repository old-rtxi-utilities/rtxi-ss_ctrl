/*
 * Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
 * Weill Cornell Medical College
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This is a template implementation file for a user module derived from
 * DefaultGUIModel with a custom GUI.
 */

#include "ss_ctrl.h"
#include <main_window.h>

extern "C" Plugin::Object*
createRTXIPlugin(void)
{
  return new SsCtrl();
}

static DefaultGUIModel::variable_t vars[] = {
  {
    "State-space controller", "Tooltip description",
    DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE,
  },


//definitely need to add reference here
	{
		"X_in","state in", DefaultGUIModel::INPUT | DefaultGUIModel::VECTORDOUBLE,
	},
	{  "r","ref", DefaultGUIModel::INPUT},
	{
		"x1","state in", DefaultGUIModel::INPUT,
	},
	{
		"x2","state in", DefaultGUIModel::INPUT,
	},
	{
		"u","stim out", DefaultGUIModel::OUTPUT,
	},



};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

SsCtrl::SsCtrl(void)
  : DefaultGUIModel("SsCtrl with Custom GUI", ::vars, ::num_vars)
{
  setWhatsThis("<p><b>SsCtrl:</b><br>QWhatsThis description.</p>");
  DefaultGUIModel::createGUI(vars,
                             num_vars); // this is required to create the GUI
  customizeGUI();
  initParameters();
  update(INIT); // this is optional, you may place initialization code directly
                // into the constructor
  refresh();    // this is required to update the GUI with parameter and state
                // values
  QTimer::singleShot(0, this, SLOT(resizeMe()));
}

SsCtrl::~SsCtrl(void)
{
}


void
SsCtrl::loadGains(void)
{
	std::string homepath = getenv("HOME");

	std::ifstream myfile;
	myfile.open(homepath+"/RTXI/modules/ss_modules/ss_ctrl/params/gain_params.txt");

	pullParamLine(myfile); //gets nx

	std::vector<double> vK = pullParamLine(myfile); 	
	Eigen::Map<Eigen::RowVector2d> tK(vK.data(),1,K.cols());
	K = tK;

	std::vector<double> nbar_vec = pullParamLine(myfile); 	
	nbar = nbar_vec[0];

	myfile.close();
}

void 
SsCtrl::printGains(void)
{
  std::cout <<"Here is the matrix K:\n" << K << "\n";
}

void SsCtrl::resetSys(void)
{
	//mostly useless? since u is set instantaneously from x which is read in from input?
    x << 0,0;
	u = 0;
}

void
SsCtrl::calcU(void)
{
	u = r*nbar-K*x;
	//setState("A State",u);
}

void
SsCtrl::execute(void)
{
  //x << input(1), input(2);
  plds::stdVec x_in = inputVector(0);
  Eigen::Vector2d x_temp(x_in.data());
  x = x_temp;//Eigen::Map<Vector2d>(x_in,1,2);//hardcode

  r = input(1);

  calcU();
  output(0) = u;

  return;
}

void
SsCtrl::initParameters(void)
{
  some_parameter = 0;
  some_state = 0;

	K << 1e2,1e2;
	x << 0,0;
	u = 0;
	loadGains();
	printGains();


}


void
SsCtrl::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag) {
    case INIT:
      period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
      setParameter("GUI label", some_parameter);
      setState("A State", some_state);
      break;

    case MODIFY:
      some_parameter = getParameter("GUI label").toDouble();
      break;

    case UNPAUSE:
      break;

    case PAUSE:
      break;

    case PERIOD:
      period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
      break;

    default:
      break;
  }
}

void
SsCtrl::customizeGUI(void)
{
  QGridLayout* customlayout = DefaultGUIModel::getLayout();

  QGroupBox* button_group = new QGroupBox;

  QPushButton* abutton = new QPushButton("Load Gains");
  QPushButton* bbutton = new QPushButton("Reset Sys");
  QHBoxLayout* button_layout = new QHBoxLayout;
  button_group->setLayout(button_layout);
  button_layout->addWidget(abutton);
  button_layout->addWidget(bbutton);
  QObject::connect(abutton, SIGNAL(clicked()), this, SLOT(aBttn_event()));
  QObject::connect(bbutton, SIGNAL(clicked()), this, SLOT(bBttn_event()));

  customlayout->addWidget(button_group, 0, 0);
  setLayout(customlayout);
}

// functions designated as Qt slots are implemented as regular C++ functions
void
SsCtrl::aBttn_event(void)
{
	loadGains();
	printGains();
}

void
SsCtrl::bBttn_event(void)
{
	resetSys();
}
