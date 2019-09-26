<!DOCTYPE html
  PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "https://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">

<head>
  <meta http-equiv="Content-Type" content="text/xhtml;charset=UTF-8" />
  <meta http-equiv="X-UA-Compatible" content="IE=9" />
  <meta name="generator" content="Doxygen 1.8.15" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Compass: install script</title>
  <link href="tabs.css" rel="stylesheet" type="text/css" />
  <script type="text/javascript" src="jquery.js"></script>
  <script type="text/javascript" src="dynsections.js"></script>
  <link href="navtree.css" rel="stylesheet" type="text/css" />
  <script type="text/javascript" src="resize.js"></script>
  <script type="text/javascript" src="navtreedata.js"></script>
  <script type="text/javascript" src="navtree.js"></script>
  <script type="text/javascript">
    /* @license magnet:?xt=urn:btih:cf05388f2679ee054f2beb29a391d25f4e673ac3&amp;dn=gpl-2.0.txt GPL-v2 */
    $(document).ready(initResizable);
/* @license-end */</script>
  <link href="search/search.css" rel="stylesheet" type="text/css" />
  <script type="text/javascript" src="search/searchdata.js"></script>
  <script type="text/javascript" src="search/search.js"></script>
  <link href="doxygen.css" rel="stylesheet" type="text/css" />
</head>

<body>
  <div id="top">
    <!-- do not remove this div, it is closed by doxygen! -->
    <div id="titlearea">
      <table cellspacing="0" cellpadding="0">
        <tbody>
          <tr style="height: 56px;">
            <td id="projectlogo"><img alt="Logo" src="compass.png" /></td>
            <td id="projectalign" style="padding-left: 0.5em;">
              <div id="projectname">Compass
              </div>
            </td>
          </tr>
        </tbody>
      </table>
    </div>
    <!-- end header part -->
    <!-- Generated by Doxygen 1.8.15 -->
    <script type="text/javascript">
      /* @license magnet:?xt=urn:btih:cf05388f2679ee054f2beb29a391d25f4e673ac3&amp;dn=gpl-2.0.txt GPL-v2 */
      var searchBox = new SearchBox("searchBox", "search", false, 'Search');
/* @license-end */
    </script>
    <script type="text/javascript" src="menudata.js"></script>
    <script type="text/javascript" src="menu.js"></script>
    <script type="text/javascript">
      /* @license magnet:?xt=urn:btih:cf05388f2679ee054f2beb29a391d25f4e673ac3&amp;dn=gpl-2.0.txt GPL-v2 */
      $(function () {
        initMenu('', true, false, 'search.php', 'Search');
        $(document).ready(function () { init_search(); });
      });
/* @license-end */</script>
    <div id="main-nav"></div>
  </div><!-- top -->
  <div id="side-nav" class="ui-resizable side-nav-resizable">
    <div id="nav-tree">
      <div id="nav-tree-contents">
        <div id="nav-sync" class="sync"></div>
      </div>
    </div>
    <div id="splitbar" style="-moz-user-select:none;" class="ui-resizable-handle">
    </div>
  </div>
  <script type="text/javascript">
    /* @license magnet:?xt=urn:btih:cf05388f2679ee054f2beb29a391d25f4e673ac3&amp;dn=gpl-2.0.txt GPL-v2 */
    $(document).ready(function () { initNavTree('d9/d7f/md_html_install_script.html', ''); });
/* @license-end */
  </script>
  <div id="doc-content">
    <!-- window showing the filter options -->
    <div id="MSearchSelectWindow" onmouseover="return searchBox.OnSearchSelectShow()"
      onmouseout="return searchBox.OnSearchSelectHide()" onkeydown="return searchBox.OnSearchSelectKey(event)">
    </div>

    <!-- iframe showing the search results (closed by default) -->
    <div id="MSearchResultsWindow">
      <iframe src="javascript:void(0)" frameborder="0" name="MSearchResults" id="MSearchResults">
      </iframe>
    </div>

    <div class="PageDoc">
      <div class="header">
        <div class="headertitle">
          <div class="title">install script </div>
        </div>
      </div>
      <!--header-->
      <div class="contents">
        <div class="textblock">
            <p><h3>setup .bashrc</h3></p>
          <div class="fragment">
            <?php
if($_POST['half16'] && intval($_POST['cuda_sm'])>=60) {
  $H16="ON";
} else {
  $H16="OFF";
}

echo '
<div class="line">## CONDA default definitions</div>
<div class="line">export CONDA_ROOT='.$_POST['conda_path'].'</div>
<div class="line">export PATH=$CONDA_ROOT/bin:$PATH</div>
<div class="line"></div>
';

if($_POST['access'] == "conda") {
  echo '
  <div class="line">#COMPASS default definitions</div>
  <div class="line">export SHESHA_ROOT=$HOME/shesha</div>
  <div class="line">export PYTHONPATH=$NAGA_ROOT:$SHESHA_ROOT:$PYTHONPATH</div>
  ';
} else {
  echo '
  <div class="line">## CUDA default definitions</div>
  <div class="line">export CUDA_ROOT='.$_POST['cuda_path'].'</div>
  <div class="line">export CUDA_INC_PATH=$CUDA_ROOT/include</div>
  <div class="line">export CUDA_LIB_PATH=$CUDA_ROOT/lib</div>
  <div class="line">export CUDA_LIB_PATH_64=$CUDA_ROOT/lib64</div>
  <div class="line">export PATH=$CUDA_ROOT/bin:$PATH</div>
  <div class="line">export LD_LIBRARY_PATH=$CUDA_LIB_PATH_64:$CUDA_LIB_PATH:$LD_LIBRARY_PATH</div>
  <div class="line">export CUDA_SM="'.$_POST['cuda_sm'].'"</div>
  <div class="line"></div>
  <div class="line">#MAGMA definitions</div>
  <div class="line">export MAGMA_ROOT="'.$_POST['magma_install'].'"</div>
  <div class="line">export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MAGMA_ROOT/lib</div>
  <div class="line">export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$MAGMA_ROOT/lib/pkgconfig</div>
  <div class="line"></div>
  <div class="line">#COMPASS default definitions</div>
  <div class="line">export COMPASS_ROOT=$HOME/compass</div>
  <div class="line">export COMPASS_INSTALL_ROOT=$COMPASS_ROOT/local</div>
  <div class="line">export COMPASS_DO_HALF="'.$H16.'" # set to ON if you want to use half precision RTC (needs SM>=60)</div>
  <div class="line">export NAGA_ROOT=$COMPASS_ROOT/naga</div>
  <div class="line">export SHESHA_ROOT=$COMPASS_ROOT/shesha</div>
  <div class="line">export LD_LIBRARY_PATH=$COMPASS_INSTALL_ROOT/lib:$LD_LIBRARY_PATH</div>
  <div class="line">export PYTHONPATH=$NAGA_ROOT:$SHESHA_ROOT:$COMPASS_INSTALL_ROOT/python:$PYTHONPATH</div>
  <div class="line">export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$COMPASS_INSTALL_ROOT/lib/pkgconfig</div>
  <div class="line"></div>
  <div class="line">#third party lib path</div>
  <div class="line">export CUB_ROOT=$COMPASS_ROOT/tplib/cub</div>
  <div class="line">export WYRM_ROOT=$COMPASS_ROOT/tplib/wyrm</div>
  ';
}
?>
          </div><!-- fragment -->

          <p><h3>script</h3></p>
          <div class="fragment">
            <?php
echo '
<div class="line">source $HOME/.bashrc</div>
<div class="line"></div>
<div class="line">mkdir -p $HOME/tmp_compass</div>
<div class="line">cd $HOME/tmp_compass</div>
<div class="line"></div>
<div class="line">wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh</div>
<div class="line">bash Miniconda3-latest-Linux-x86_64.sh -b -p $CONDA_ROOT</div>
<div class="line"></div>
';

if($_POST['access'] == "conda") {
  echo '
  <div class="line">conda install -y -c compass compass</div>
  <div class="line">cd $HOME</div>
  <div class="line">git clone https://github.com/ANR-COMPASS/shesha.git</div>
  ';
} else {
  echo '
  <div class="line">export MKLROOT=$CONDA_ROOT</div>
  <div class="line">export CUDADIR=$CUDA_ROOT </div>
  <div class="line">export NCPUS=8</div>
  <div class="line">export GPU_TARGET=sm_$CUDA_SM </div>
  <div class="line"></div>
  <div class="line">conda install -y numpy mkl-include pyqtgraph ipython pyqt qt matplotlib astropy blaze h5py hdf5 pytest pandas scipy docopt tqdm</div>
  <div class="line"></div>
  <div class="line">wget http://icl.cs.utk.edu/projectsfiles/magma/downloads/magma-2.5.0.tar.gz -O - | tar xz</div>
  <div class="line">cd magma-2.5.0</div>
  <div class="line"></div>
  <div class="line">cp make.inc-examples/make.inc.mkl-gcc make.inc</div>
  <div class="line">sed -i -e "s:/intel64: -Wl,-rpath=$CUDADIR/lib64 -Wl,-rpath=$MKLROOT/lib:" make.inc</div>
  <div class="line"></div>
  <div class="line">make -j $NCPUS shared sparse-shared</div>
  <div class="line">make install prefix=$MAGMA_ROOT</div>
  <div class="line"></div>
  <div class="line">cd $HOME</div>
  <div class="line">git clone https://gitlab.obspm.fr/compass/compass --recurse-submodules</div>
  <div class="line">cd $COMPASS_ROOT</div>
  <div class="line">./compile.sh</div>
  ';
}
?>
          </div><!-- fragment -->
        </div><!-- textblock -->
      </div><!-- contents -->
    </div><!-- PageDoc -->
  </div><!-- doc-content -->
  <!-- start footer part -->
  <div id="nav-path" class="navpath">
    <!-- id is needed for treeview function! -->
    <ul>
      <li class="footer">Generated by
        <a href="http://www.doxygen.org/index.html">
          <img class="footer" src="doxygen.png" alt="doxygen" /></a> 1.8.15 </li>
    </ul>
  </div>
</body>

</html>
