using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace HomeDeck
{
    /// <summary>
    /// Interaction logic for PropertiesWnd.xaml
    /// </summary>
    public partial class PropertiesWnd : Window
    {
        MainWindow m_parentWnd = null;
        MainWindow.OUT_OBJECTINFO m_pSelObjectData = new MainWindow.OUT_OBJECTINFO();
        int m_nSelectedObject;



        public PropertiesWnd()
        {
            InitializeComponent();
        }

        public PropertiesWnd(MainWindow parent)
        {
            InitializeComponent();
            m_parentWnd = parent;
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            String[] attrName = new String[] { "x", "y", "z", "xa", "name" };
            MainWindow.GetGroupNextInfo(attrName);
            /*
            int nParent = -1;
            string strParentId = "";
            MainWindow.OUT_OBJECTINFO pParent = new MainWindow.OUT_OBJECTINFO();

            //Get Selected Object
            m_nSelectedObject = MainWindow.GetSelectedObject(ref m_pSelObjectData);
            if (m_nSelectedObject < 0)
                return;
            
            //Get ID and Display
            txtID.Text = m_pSelObjectData.szId;
            
            //Get ID of Parents and Display
            nParent = MainWindow.GetParentObject(m_nSelectedObject, ref pParent);
            while (nParent > 0) {
                if (strParentId != "")
                    strParentId += " -> ";
                strParentId += pParent.szId;
                nParent = MainWindow.GetParentObject(nParent, ref pParent);
            }
            txtParentID.Text = strParentId; */
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        
    }
}
