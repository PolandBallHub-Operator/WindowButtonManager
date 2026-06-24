# Titlebar button manage NOADMIN
<p align="center">
  <img src="https://raw.githubusercontent.com/PolandBallHub-Operator/WindowButtonManager/refs/heads/main/wtm.png" alt="WBM2" width="500">
</p>
このツールは、Windowsの標準TitlebarbuttonをMac風の信号機ボタンやカスタム画像に置き換える（オーバーレイする）ための管理ツールです。

## 特徴
- **複数ウィンドウ対応**: アクティブなウィンドウだけでなく、表示されているすべてのウィンドウに個別に追従します。
- **Mac風信号機デフォルト**: 起動直後からMac OSのような赤・黄・緑のボタンが表示されます。
- **NOADMIN（管理者権限不要）**: システム設定を変更せず、一般ユーザー権限で動作します。
- **GUI設定**: ボタンのサイズ、位置オフセット、間隔をGUIからリアルタイムで調整可能です。

## 内容物
- `TitlebarButtonManager.exe`: アプリ本体
- `assets/`: デフォルトのボタン画像
- `main.cpp`: ソースコード
- `config.ini`: 設定ファイル（初回保存時に生成されます）

## 使い方
1. `TitlebarButtonManager.exe` を実行します。
2. 設定画面（Window button manage NOADMIN）が表示されます。
3. 以下の項目を調整し、「Apply & Save」をクリックすると反映されます：
   - **Button Size**: 各ボタンの大きさ（ピクセル）
   - **Offset X / Y**: ウィンドウ右上からの表示位置の微調整
4. カスタム画像を使用したい場合は、`assets/` フォルダ内の画像を同名のファイルで上書きしてください。

## 終了方法
設定画面のウィンドウを閉じると、すべてのオーバーレイと共にアプリが終了します。

## 技術仕様
- 言語: C++ (Win32 API / GDI+)
- ビルド: MinGW-w64 (Static Link)
- 依存DLL: なし（標準システムDLLのみ）

*WBMはwindows boot managerのため、TBMに変更
