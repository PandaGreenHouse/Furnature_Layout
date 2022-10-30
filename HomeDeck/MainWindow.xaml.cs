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
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Interop;
using System.Runtime.InteropServices;
using System.Windows.Controls.Primitives;
using System.IO;
using Microsoft;

namespace HomeDeck
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
//Structures

        //Material Type Constants
        public const int MAT_TYPE_COLOR = 0;
        public const int MAT_TYPE_IMAGE = 1;
        public const int XML_MAX_PATH_LEN = 512;
        public const int MAX_OBJECT_NAME_LEN = 40;

        [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct OUT_MATINFO
        {
            [MarshalAsAttribute(UnmanagedType.I4)]
            public int    nType;

            [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst=4)]
            public float[] clrAmbient;
            [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 4)]
            public float[] clrDiffuse;
            [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 4)]
            public float[] clrSpecular;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = XML_MAX_PATH_LEN)]
            public string szImgPath;
        }

        [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct OUT_OBJECTINFO
        {
            [MarshalAsAttribute(UnmanagedType.I4)]
            public int nType;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = MAX_OBJECT_NAME_LEN)]
            public string szId;
            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = MAX_OBJECT_NAME_LEN)]
            public string szName;
            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = MAX_OBJECT_NAME_LEN)]
            public string szCode;
            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = MAX_OBJECT_NAME_LEN)]
            public string szModel;

            [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 3)]
            public float[] fPos;    //x, y, z
            [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 3)]
            public float[] fSize;   //w, d, h
            [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 3)]
            public float[] fRot;    //xa, ya, za
        }

//DLL Functions
        [DllImport("RenderView.dll", EntryPoint = "SetViewMode")]
        private static extern void SetViewMode(int nMode);

        [DllImport("RenderView.dll", EntryPoint = "LoadModel", CharSet = CharSet.Unicode)]
        public static extern int LoadModel([param: MarshalAs(UnmanagedType.LPTStr)] string fileName, [param: MarshalAs(UnmanagedType.LPTStr)] string fileContent);

        [DllImport("RenderView.dll", EntryPoint = "LoadModelByContent", CharSet = CharSet.Unicode)]
        public static extern int LoadModelByContent([param: MarshalAs(UnmanagedType.LPTStr)] string fileContent);

        [DllImport("RenderView.dll", EntryPoint = "SaveModel", CharSet = CharSet.Unicode)]
        public static extern int SaveModel([param: MarshalAs(UnmanagedType.LPTStr)] string fileName);

        [DllImport("RenderView.dll", EntryPoint = "GetMaterialCount", CharSet = CharSet.Unicode)]
        public static extern int GetMaterialCount();

        [DllImport("RenderView.dll", EntryPoint = "GetMaterialInfo", CharSet = CharSet.Unicode)]
	    public static extern int  GetMaterialInfo(int nIndex, out OUT_MATINFO pMat);

        [DllImport("RenderView.dll", EntryPoint = "AddMaterial", CharSet = CharSet.Unicode)]
	    public static extern int  AddMaterial(ref OUT_MATINFO pMat);

        [DllImport("RenderView.dll", EntryPoint = "EditMaterial", CharSet = CharSet.Unicode)]
	    public static extern int  EditMaterial(int nIndex, ref OUT_MATINFO pMat);

        //Return Values: 0 - Delete failed, 1 - Delete Success	2 - Cannot Delete because some objects use this material
        [DllImport("RenderView.dll", EntryPoint = "DeleteMaterial", CharSet = CharSet.Unicode)]
	    public static extern int  DeleteMaterial(int nIndex);

        [DllImport("RenderView.dll", EntryPoint = "ApplyTexture", CharSet = CharSet.Unicode)]
	    public static extern int  ApplyTexture(int nObject, int nMatIdx);

        [DllImport("RenderView.dll", EntryPoint = "SetLineDrawFlag")]
        public static extern void SetLineDrawFlag(int nFlag);

        [DllImport("RenderView.dll", EntryPoint = "RotateObject")]
        public static extern void RotateObject(int nDegree);

        [DllImport("RenderView.dll", EntryPoint = "GetSelectedObject")]
        public static extern int GetSelectedObject(ref OUT_OBJECTINFO pObj);

        [DllImport("RenderView.dll", EntryPoint = "GetParentObject")]
        public static extern int GetParentObject(int nObject, ref OUT_OBJECTINFO pObj);

        [DllImport("RenderView.dll", EntryPoint = "EditSelectedObject")]
        public static extern int EditSelectedObject(int nObject, ref OUT_OBJECTINFO pObj);

        [DllImport("RenderView.dll", EntryPoint = "GetGroupNextInfo", CharSet = CharSet.Unicode)]
        public static extern IntPtr GetGroupNextInfo(String[] attrName, int nCount);

        [DllImport("RenderView.dll", EntryPoint = "ReleaseString", CharSet = CharSet.Unicode)]
        public static extern void ReleaseString(IntPtr str);

        [DllImport("RenderView.dll", EntryPoint = "ScatterObjects")]
        public static extern void ScatterObjects(int nCount);

        [DllImport("RT_Render.dll", EntryPoint = "GI_Render")]
        public static extern void GI_Render();

        public static List<String> GetGroupNextInfo(String[] attrName)
        {
            IntPtr res;
            String[] arr;
            List<String> list = null;

            res = GetGroupNextInfo(attrName, attrName.Length);
            if (res != IntPtr.Zero)
            {
                arr  = Marshal.PtrToStringUni(res).Split('\n');
                list = new List<String>(arr);
                ReleaseString(res);
            }

            return list;
        }

//Global Variable Definitions
        //View Mode constants equals at RenderView Dll
        public static int VIEW_MODE_MIN         = 0;
        public static int VIEW_MODE_MAX         = 6;

        public static int VIEW_MODE_FOUR         = 0;
        public static int VIEW_MODE_AIR         = 1;
        public static int VIEW_MODE_FRONT       = 2;
        public static int VIEW_MODE_SIDE        = 3;
        public static int VIEW_MODE_PERSPECTIVE = 4;
        public static int VIEW_MODE_WIREFRAME   = 5;
        public static int VIEW_MODE_GEOMETRY = 6;
        

        //View Mode and its' Check Buttons
        ToggleButton[] m_chkViewModes = new ToggleButton[VIEW_MODE_MAX + VIEW_MODE_MIN + 1];
        int m_nCurViewMode = VIEW_MODE_FOUR;

        //Material UI
        const int MAT_COUNT_PER_PAGE = 9;
        Canvas[] m_matSamples = new Canvas[MAT_COUNT_PER_PAGE];
        int[] m_matIndices = new int[MAT_COUNT_PER_PAGE];

        static Color DEF_MAT_IMG_BORDER = Color.FromRgb(0, 0, 0);
        static Color SEL_MAT_IMG_BORDER = Color.FromRgb(0, 255, 0);
        static Color MAT_DEF_BACK_COLOR = Color.FromRgb(51, 51, 51);
        static Color MAT_NOIMG_COLOR    = Color.FromRgb(128, 128, 128); //Color that texture image is not exists

        List<OUT_MATINFO> m_lstMats = new List<OUT_MATINFO>();
        int m_nCurMatPage = 0;          //Current Material List Page No.
        int m_nMatPageCount = 0;        //Material List Page Count
        int m_nSelMatCanvas = -1;       //Selected Material Canvas Index (at 9 canvas list)

        bool m_bModelFileLoaded = false;

        int m_nScatter; //Scattering Slider Value


        public MainWindow()
        {
            InitializeComponent();

            //Viewport Check Box Array
            m_chkViewModes[VIEW_MODE_FOUR]          = chkFourView;
            m_chkViewModes[VIEW_MODE_AIR]           = chkAirView;
            m_chkViewModes[VIEW_MODE_FRONT]         = chkFrontView;
            m_chkViewModes[VIEW_MODE_SIDE]          = chkSideView;
            m_chkViewModes[VIEW_MODE_PERSPECTIVE]   = chkPerspectiveView;
            m_chkViewModes[VIEW_MODE_WIREFRAME]     = chkWireframeView;
            m_chkViewModes[VIEW_MODE_GEOMETRY]      = chkGeometryView;

            //Material UI
            m_matSamples[0] = matSample1;
            m_matSamples[1] = matSample2;
            m_matSamples[2] = matSample3;
            m_matSamples[3] = matSample4;
            m_matSamples[4] = matSample5;
            m_matSamples[5] = matSample6;
            m_matSamples[6] = matSample7;
            m_matSamples[7] = matSample8;
            m_matSamples[8] = matSample9;
            for (int x = 0; x < MAT_COUNT_PER_PAGE; x++)
                m_matIndices[x] = -1;

            //Scattering Slider
            sldScatter.Minimum = 0;
            sldScatter.Maximum = 50;
            sldScatter.Value = 0;
            m_nScatter = 0;
            
            //Enable Controls by Initial state
            EnableControls();
        }

        private void OnSourceInitialized(object sender, EventArgs e)
        {
            Window wnd = sender as Window;
            wnd.WindowStartupLocation = WindowStartupLocation.Manual;
            wnd.Left = 0; wnd.Top = 0;
            wnd.Width   = SystemParameters.WorkArea.Width;
            wnd.Height  = SystemParameters.WorkArea.Height;
            wnd.MaxWidth    = SystemParameters.WorkArea.Width;
            wnd.MaxHeight   = SystemParameters.WorkArea.Height;
            //wnd.WindowState = WindowState.Maximized;
            wnd.SizeToContent = SizeToContent.Manual;
        }

        private void OnLoaded(object sender, EventArgs e)
        {
            Window wndMain;
            RenderViewHwndHost rh;

            wndMain = System.Windows.Application.Current.MainWindow;
            rh = new RenderViewHwndHost(wndMain, RenderViewHwndHost.NOTIFIER_TYPE_WINDOW);
            wndRender.Child = rh;
        }

        private string ExtractFileNameFromPath(string path)
        {
            string name = "";
            int nIndex = path.LastIndexOf('\\');
            if (nIndex < 0)
                nIndex = path.LastIndexOf('/');
            if (nIndex < 0)
                name = path;
            name = path.Substring(nIndex + 1);
            return name;
        }

//Menu Handlers
        private void onClick_btnMenu(object sender, RoutedEventArgs e)
        {
            OpenContextMenu(sender as FrameworkElement);
        }

        private void OpenContextMenu(FrameworkElement elem)
        {
            if (elem.ContextMenu != null)
            {
                elem.ContextMenu.PlacementTarget = elem;
                elem.ContextMenu.Placement = PlacementMode.Bottom;
                elem.ContextMenu.IsOpen = true;
            }
        }

   //File Menu
        private void onMenuClick_FileOpen(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            // Set filter for file extension and default file extension 
            dlg.DefaultExt = ".xml";
            dlg.Filter = "Model Files (*.xml)|*.xml";
            // Display OpenFileDialog by calling ShowDialog method 
            Nullable<bool> result = dlg.ShowDialog();
            // Get the selected file name and display in a TextBox 
            if (result == true)
            {
                //Get File Name
                string filename = dlg.FileName;
                
                //Open File
                m_bModelFileLoaded = false;
                if (LoadModel(filename, "") != 0)
                {
                    Title = "모형: " + ExtractFileNameFromPath(filename);  //Set Window Title
                    chkViewMode_Click(VIEW_MODE_FOUR);
                    m_bModelFileLoaded = true;
                    LoadMaterials();
                    wndRender.Visibility = Visibility.Visible;
                }
            }
        }

        private void LoadMaterials()
        {
            //Get Material Info
            int nCount, x;
            OUT_MATINFO mat;

            m_lstMats.Clear();
            nCount = GetMaterialCount();
            for (x = 0; x < nCount; x++)
            {
                mat = new OUT_MATINFO();
                mat.szImgPath = new string('\0', 512);
                GetMaterialInfo(x, out mat);
                m_lstMats.Add(mat);
            }
            UpdateMaterialList();
            PreviewMaterial(-1);
        }

        private void onMenuClick_FileSave(object sender, RoutedEventArgs e)
        {
            if (!m_bModelFileLoaded)
                return;
            Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();
            // Set filter for file extension and default file extension 
            dlg.DefaultExt = ".xml";
            dlg.Filter = "Model Files (*.xml)|*.xml";
            // Display OpenFileDialog by calling ShowDialog method 
            Nullable<bool> result = dlg.ShowDialog();
            // Get the selected file name and display in a TextBox 
            if (result == true)
            {
                //Get File Name
                string filename = dlg.FileName;
                //Save File
                if (SaveModel(filename) != 0)
                    Title = "Model: " + ExtractFileNameFromPath(filename);  //Set Window Title
            }
        }

        private void onMenuClick_FileExit(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

    //Edit Menu
        private void onMenu_EditOpened(object sender, RoutedEventArgs e)
        {
            MenuItem sm = sender as MenuItem;
            MenuItem mat = sm.Items[0] as MenuItem; //Materials.. Item
            //mat.IsEnabled = false;
        }

        private void onMenuClick_EditMaterials(object sender, RoutedEventArgs e)
        {

        }

    //View Menu
        private void onMenu_ViewOpened(object sender, RoutedEventArgs e)
        {
            MenuItem sm = sender as MenuItem;
            for (int x = VIEW_MODE_MIN; x <= VIEW_MODE_MAX; x++)
            {
                MenuItem it = sm.Items[x - VIEW_MODE_MIN] as MenuItem;
                it.IsChecked = (m_nCurViewMode == x) ? true : false;
            }
        }

//Button Handlers
    //Toolbar Buttons
        //Rotate 90 Button
        private void btnRotate90_Click(object sender, RoutedEventArgs e)
        {
            RotateObject(90);
        }
        //Line Draw CheckBox
        private void chkLineDraw_Click(object sender, RoutedEventArgs e)
        {
            SetLineDrawFlag((chkLineDraw.IsChecked == true) ? 1 : 0);
        }


        //View Check Buttons
        private void chkViewMode_Click(int nViewMode)
        {
            if (m_nCurViewMode == nViewMode)
            {
                m_chkViewModes[m_nCurViewMode].IsChecked = true;
                return;
            }

            m_nCurViewMode = nViewMode;
            for (int x = VIEW_MODE_MIN; x <= VIEW_MODE_MAX; x++)
                m_chkViewModes[x].IsChecked = false;
            m_chkViewModes[m_nCurViewMode].IsChecked = true;
            SetViewMode(nViewMode);
        }

        //四格图
        private void chkFourView_Click(object sender, RoutedEventArgs e)
        {
            chkViewMode_Click(VIEW_MODE_FOUR);
        }

        //俯视图
        private void chkAirView_Click(object sender, RoutedEventArgs e)
        {
            chkViewMode_Click(VIEW_MODE_AIR);
        }
        //正视图
        private void chkFrontView_Click(object sender, RoutedEventArgs e)
        {
            chkViewMode_Click(VIEW_MODE_FRONT);
        }
        //侧视图
        private void chkSideView_Click(object sender, RoutedEventArgs e)
        {
            chkViewMode_Click(VIEW_MODE_SIDE);
        }
        //三维图
        private void chkPerspectiveView_Click(object sender, RoutedEventArgs e)
        {
            chkViewMode_Click(VIEW_MODE_PERSPECTIVE);
        }
        
        //三维透视图
        private void chkGeometryView_Click(object sender, RoutedEventArgs e)
        {
            chkViewMode_Click(VIEW_MODE_GEOMETRY);
        }
        
        //三维线框图
        private void chkWireframeView_Click(object sender, RoutedEventArgs e)
        {
            chkViewMode_Click(VIEW_MODE_WIREFRAME);
        }

        //Material Pane Buttons
        //Shows Previous 9 Materials
        private void OnClick_BtnMatPrevPage(object sender, RoutedEventArgs e)
        {
            if (m_nCurMatPage > 0)
                m_nCurMatPage--;
            UpdateMaterialList();
        }

        //Shows Next 9 Materials
        private void OnClick_BtnMatNextPage(object sender, RoutedEventArgs e)
        {
            if (m_nCurMatPage < (m_nMatPageCount - 1))
                m_nCurMatPage++;
            UpdateMaterialList();
        }

        //Draw Material Image for a Canvas of 9 canvases
        private void setCanvasMaterial(int nCanvas)
        {
            int nMatIdx = -1;
            Canvas current = null;
            OUT_MATINFO mat;

            //Get Current Canvas
            current = m_matSamples[nCanvas];
            nMatIdx = m_matIndices[nCanvas];
            if (nMatIdx >= 0)
            {
                //Get Current Material Info
                mat = m_lstMats[nMatIdx];
                //Set Canvas Background from Material Info
                switch (mat.nType)
                {
                case MAT_TYPE_COLOR:
                    current.Background = new SolidColorBrush(Color.FromRgb((byte) ((int) mat.clrDiffuse[0]), 
                                            (byte) ((int) mat.clrDiffuse[1]), (byte) ((int) mat.clrDiffuse[2])));
                    break;

                case MAT_TYPE_IMAGE:
                    if (mat.szImgPath == "")
                        current.Background = new SolidColorBrush(MAT_NOIMG_COLOR);
                    else if (File.Exists(mat.szImgPath) == false) //File not Exist
                        current.Background = new SolidColorBrush(MAT_NOIMG_COLOR);
                    else
                        current.Background = new ImageBrush(new BitmapImage(new Uri(mat.szImgPath, UriKind.Relative)));
                    break;
                }
            } else
                current.Background = new SolidColorBrush(MAT_DEF_BACK_COLOR);
        }

        //Update Material Display List
        private void UpdateMaterialList()
        {
            int nStart, nEnd, x;

            //Get Page Count
            m_nMatPageCount = (m_lstMats.Count + MAT_COUNT_PER_PAGE - 1) / MAT_COUNT_PER_PAGE;
            //Enable Buttons
            EnableControls();

            //Page Text
            if (m_nMatPageCount > 0)
                matPageNo.Content = (m_nCurMatPage + 1).ToString() + " / " + m_nMatPageCount.ToString();
            else
                matPageNo.Content = "";

            //Set Selected Canvas
            nStart = m_nCurMatPage * MAT_COUNT_PER_PAGE;

            //Setting 9 Canvases
            nEnd = nStart + MAT_COUNT_PER_PAGE;
            if (nEnd > m_lstMats.Count)
                nEnd = m_lstMats.Count;
            for (x = nStart; x < nEnd; x++)
                m_matIndices[x - nStart] = x;
            for (x = nEnd - nStart; x < MAT_COUNT_PER_PAGE; x++)
                m_matIndices[x] = -1;

            for (x = 0; x < MAT_COUNT_PER_PAGE; x++)
                setCanvasMaterial(x);
        }

        //Material Select
        void SelectMaterial(int nMatIdx)
        {
            int nPage = (int)(nMatIdx / MAT_COUNT_PER_PAGE), x;
            int nFirst, nLast = 0;

            nFirst = nPage * MAT_COUNT_PER_PAGE;
            nLast = nFirst + MAT_COUNT_PER_PAGE;
            for (x = nFirst; x < nLast; x++)
                m_matIndices[x - nFirst] = (x >= m_lstMats.Count) ? -1 : nFirst;

            m_nSelMatCanvas = nMatIdx - nFirst;
            UpdateMaterialList();
            PreviewMaterial(m_nSelMatCanvas);
        }

        //Add Material, Shows Dialog to Add Material
        private void OnClick_BtnMatAdd(object sender, RoutedEventArgs e)
        {
            OUT_MATINFO sInfo = new OUT_MATINFO();

            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            // Set filter for file extension and default file extension 
            dlg.DefaultExt = ".jpg";
            dlg.Filter = "Texture Image Files (*.jpg, *.png, *.gif)|*.jpg;*.png;*.gif;";
            // Display OpenFileDialog by calling ShowDialog method 
            Nullable<bool> result = dlg.ShowDialog();
            // Get the selected file name and display in a TextBox 
            if (result == true)
            {
                //Image Document
                sInfo = new OUT_MATINFO();
                sInfo.szImgPath = dlg.FileName;
                sInfo.nType = MAT_TYPE_IMAGE;
                m_lstMats.Add(sInfo);
                AddMaterial(ref sInfo);
                SelectMaterial(m_lstMats.Count - 1);
            }
        }
        //Delete Material, Confirms first
        private void OnClick_BtnMatDelete(object sender, RoutedEventArgs e)
        {
            if ((m_nSelMatCanvas < 0) || (m_nSelMatCanvas >= m_lstMats.Count))
                return;
            if ((m_matIndices[m_nSelMatCanvas] < 0) || (m_matIndices[m_nSelMatCanvas] >= m_lstMats.Count))
                return;

            //MessageBoxResult result = MessageBox.Show("Do you really want Delete?", "Confirm", MessageBoxButton.YesNo);
            //if (result == MessageBoxResult.Yes)
            {
                m_lstMats.RemoveAt(m_matIndices[m_nSelMatCanvas]);
                switch (DeleteMaterial(m_matIndices[m_nSelMatCanvas]))
                {
                    case 2:
                        //MessageBox.Show("Cannot be deleted because some objects use this material.");
                        break;
                    case 1:
                        m_nSelMatCanvas = 0;
                        SelectMaterial(0);
                        break;
                }
            }
        }
        //Edit Material, Shows Dialog to Edit Material
        private void OnClick_BtnMatEdit(object sender, RoutedEventArgs e)
        {
            if ((m_nSelMatCanvas < 0) || (m_nSelMatCanvas >= m_lstMats.Count))
                return;
            if ((m_matIndices[m_nSelMatCanvas] < 0) || (m_matIndices[m_nSelMatCanvas] >= m_lstMats.Count))
                return;

            OUT_MATINFO sInfo = new OUT_MATINFO();
            int nMatIdx = m_matIndices[m_nSelMatCanvas];

            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            // Set filter for file extension and default file extension 
            dlg.DefaultExt = ".jpg";
            dlg.Filter = "Texture Image Files (*.jpg, *.png, *.gif)|*.jpg;*.png;*.gif";
            // Display OpenFileDialog by calling ShowDialog method 
            Nullable<bool> result = dlg.ShowDialog();
            // Get the selected file name and display in a TextBox 
            if (result == true)
            {
                sInfo = m_lstMats[nMatIdx];
                //Image Document
                sInfo.szImgPath = dlg.FileName;
                sInfo.nType = MAT_TYPE_IMAGE;

                m_lstMats.RemoveAt(nMatIdx);
                if (nMatIdx >= m_lstMats.Count)
                    m_lstMats.Add(sInfo);
                else
                    m_lstMats.Insert(nMatIdx, sInfo);
                EditMaterial(nMatIdx, ref sInfo);
                SelectMaterial(nMatIdx);
            }
        }

        //Apply Material to Object
        private void OnClick_BtnMatApply(object sender, RoutedEventArgs e)
        {
            if (m_nSelMatCanvas < 0 || m_nSelMatCanvas >= m_lstMats.Count)
                return;
            if ((m_matIndices[m_nSelMatCanvas] < 0) || (m_matIndices[m_nSelMatCanvas] >= m_lstMats.Count))
                return;

            //Get U,V tiling values
            //float tu, tv;

            //tu = (float)Convert.ToDouble(txtUTiling.Text);
            //tv = (float)Convert.ToDouble(txtVTiling.Text);


            //Apply Material
            ApplyTexture(0, m_matIndices[m_nSelMatCanvas]);
        }

        private void PreviewMaterial(int nCanvas)
        {
            int nMatIdx = -1;
            OUT_MATINFO mat;

            m_nSelMatCanvas = nCanvas;
            if (nCanvas < 0)
            {
                matPreview.Background = new SolidColorBrush(MAT_DEF_BACK_COLOR);
                return;
            }

            nMatIdx = m_matIndices[nCanvas];
            if (nMatIdx >= 0)
            {
                //Get Current Material Info
                mat = m_lstMats[nMatIdx];
                switch (mat.nType)
                {
                case MAT_TYPE_COLOR:
                    matPreview.Background = new SolidColorBrush(Color.FromRgb((byte)((int)mat.clrDiffuse[0]),
                                                (byte)((int)mat.clrDiffuse[1]), (byte)((int)mat.clrDiffuse[2])));
                    break;

                case MAT_TYPE_IMAGE:
                    //Set Canvas Background from Material Info
                    if (mat.szImgPath == "")
                        matPreview.Background = new SolidColorBrush(MAT_NOIMG_COLOR);
                    else if (File.Exists(mat.szImgPath) == false) //File not Exist
                        matPreview.Background = new SolidColorBrush(MAT_NOIMG_COLOR);
                    else
                        matPreview.Background = new ImageBrush(new BitmapImage(new Uri(mat.szImgPath, UriKind.Relative)));
                    break;
                }
            }
            else
                matPreview.Background = new SolidColorBrush(MAT_DEF_BACK_COLOR);
        }

        private void MatSample_Click(int index)
        {
            PreviewMaterial(index);
            EnableControls();
        }

        private void MatSample1_MouseDown(object sender, MouseButtonEventArgs e)
        {
            MatSample_Click(0);
        }

        private void MatSample2_MouseDown(object sender, MouseButtonEventArgs e)
        {
            MatSample_Click(1);
        }

        private void MatSample3_MouseDown(object sender, MouseButtonEventArgs e)
        {
            MatSample_Click(2);
        }

        private void MatSample4_MouseDown(object sender, MouseButtonEventArgs e)
        {
            MatSample_Click(3);
        }

        private void MatSample5_MouseDown(object sender, MouseButtonEventArgs e)
        {
            MatSample_Click(4);
        }

        private void MatSample6_MouseDown(object sender, MouseButtonEventArgs e)
        {
            MatSample_Click(5);
        }

        private void MatSample7_MouseDown(object sender, MouseButtonEventArgs e)
        {
            MatSample_Click(6);
        }

        private void MatSample8_MouseDown(object sender, MouseButtonEventArgs e)
        {
            MatSample_Click(7);
        }

        private void MatSample9_MouseDown(object sender, MouseButtonEventArgs e)
        {
            MatSample_Click(8);
        }

//Enable/Disable Controls
        public void EnableControls()
        {
            btnMatPrevPage.IsEnabled = (m_nCurMatPage > 0) ? true : false;
            btnMatNextPage.IsEnabled = (m_nCurMatPage < (m_nMatPageCount - 1)) ? true : false;

        }

//RenderView Window Procedure Hooking Function
        public IntPtr RenderViewWndProc(int msg, IntPtr wParam, IntPtr lParam)
        {
            switch (msg)
            {
                case 0x204: //WM_RBUTTONDOWN:
                    wndRender.ContextMenu.IsOpen = true;
                    break;
            }
            return IntPtr.Zero;
        }

        private void onPopupMenu_Properties(object sender, RoutedEventArgs e)
        {
            //if (!m_bModelFileLoaded)
            //  return;
            PropertiesWnd pw = new PropertiesWnd(this);
                        
            pw.ShowDialog();
        }

        private void onPopupMenu_Guide(object sender, RoutedEventArgs e)
        {
            if (!m_bModelFileLoaded)
                return;

            PropertiesWnd pw = new PropertiesWnd(this);

            pw.ShowDialog();
        }

        private void onPopupMenu_Delete(object sender, RoutedEventArgs e)
        {
            if (!m_bModelFileLoaded)
                return;
            PropertiesWnd pw = new PropertiesWnd(this);

            pw.ShowDialog();
        }

        private void onPopupMenu_Hide(object sender, RoutedEventArgs e)
        {
            if (!m_bModelFileLoaded)
                return;
            PropertiesWnd pw = new PropertiesWnd();

            pw.ShowDialog();
        }

        private void onPopupMenu_Rotate90(object sender, RoutedEventArgs e)
        {
            RotateObject( 90);
        }

        private void onPopupMenu_Rotate180(object sender, RoutedEventArgs e)
        {
            RotateObject(180);
        }

        private void onPopupMenu_Rotate270(object sender, RoutedEventArgs e)
        {
            RotateObject(270);
        }


        //Handling Event of Scattering Slider
        private void sldScatter_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            ScatterObjects((int) sldScatter.Value - m_nScatter);
            m_nScatter = (int) sldScatter.Value;
        }

        private void RT_Render(object sender, RoutedEventArgs e)
        {
            GI_Render();
        }
    }

    public class RenderViewHwndHost : HwndHost
    {
        [DllImport("RenderView.dll", EntryPoint = "CreateRenderWindow", CharSet = CharSet.Unicode)]
        private static extern IntPtr CreateRenderWindow(IntPtr hInstance, IntPtr hWndParent, int nWidth, int nHeight, int nLeft, int nTop);

        [DllImport("RenderView.dll", EntryPoint = "DestroyRenderWindow")]
        private static extern void DestroyRenderWindow();

        [DllImport("RenderView.dll", EntryPoint = "ShowHideRenderView")]
        private static extern void ShowHideRenderView(int bShow);

        public const int NOTIFIER_TYPE_WINDOW = 0;
        public const int NOTIFIER_TYPE_CONTROL = 1;

        int m_nNotifierType;        //Event Notifier Type
        Window m_wndNotify;         //Event Notifier of Window Type
        UserControl m_ctrlNotify;   //Event Notifier of User Control
        Window m_wndParent;

        public RenderViewHwndHost(Object notifier, int type)
        {
            //Set Visibility Change Event Handler
            this.IsVisibleChanged += RenderViewHost_IsVisibleChanged;
            //Set up Notifier
            m_nNotifierType = type;
            switch (type)
            {
                case NOTIFIER_TYPE_WINDOW:
                    m_wndNotify = notifier as Window;
                    break;
                case NOTIFIER_TYPE_CONTROL:
                    m_ctrlNotify = notifier as UserControl;
                    break;
            }

        }

        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            //Get Parent Window and Root Window
            Border wndParent = this.Parent as Border;
            Window rootWnd = Application.Current.MainWindow;
            //Get Left-Top Position of Parent Window
            Point ptTop = wndParent.PointToScreen(new Point(0, 0));
            //Offset from Root Window
            ptTop = rootWnd.PointFromScreen(ptTop);
            //Adjust Border Size
            ptTop.X += wndParent.BorderThickness.Left;
            ptTop.Y += wndParent.BorderThickness.Top;
            //Get Size of Parent Window
            Point ptSize = new Point(wndParent.ActualWidth, wndParent.ActualHeight);
            ptSize.X -= (wndParent.BorderThickness.Left + wndParent.BorderThickness.Right);
            ptSize.Y -= (wndParent.BorderThickness.Top + wndParent.BorderThickness.Bottom);
            //Get Instance Handle
            IntPtr instanceHandle = System.Runtime.InteropServices.Marshal.GetHINSTANCE(System.Reflection.Assembly.GetExecutingAssembly().GetModules()[0]);
            //Create Window by DLL function
            IntPtr hwndHost = CreateRenderWindow(instanceHandle, hwndParent.Handle, (int)ptSize.X, (int)ptSize.Y, (int)ptTop.X, (int)ptTop.Y);
            //Show/Hide Window according to Visibility of Parent Window
            ShowHideRenderView(wndParent.IsVisible ? 1 : 0);
            //Return the Handle of Newly Created Window
            return new HandleRef(this, hwndHost);
        }

        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            DestroyRenderWindow();
        }

        private void RenderViewHost_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            ShowHideRenderView(this.IsVisible ? 1 : 0);
        }

        protected override IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            handled = false;

            switch (msg)
            {
            case 0x002: //WM_DESTROY:
                 break;
            case 0x204: //WM_RBUTTONDOWN:
                 //Please Modify this as your need
                 switch (m_nNotifierType)
                 {
                     case NOTIFIER_TYPE_WINDOW:
                         if (m_wndNotify != null)
                             (m_wndNotify as MainWindow).RenderViewWndProc(msg, wParam, lParam);
                         break;
                     case NOTIFIER_TYPE_CONTROL:
                         break;
                 }
                break;
            }
            return IntPtr.Zero;
        }
    }
}
