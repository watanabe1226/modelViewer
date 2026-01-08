# DirectX 12 Model Viewer

DirectX 12 APIを用いたリアルタイム3Dモデルビューアーです。

<img width="1914" height="1014" alt="スクリーンショット 2026-01-08 222644" src="https://github.com/user-attachments/assets/0d93e218-62ac-46a9-b445-fdbb2272a686" />


## 主な機能 (Features)

### 1. Image-Based Lighting (IBL)
* **環境光によるレンダリング**: 放射輝度マップと鏡面反射マップを使用したライティング

### 2. Shadow Mapping
* **動的な影の生成**: ライト視点の深度情報をデプスバッファに書き込み、シャドウマップとして利用

### 3. glTF Model Loading
* glTF形式のモデル読み込み

## 技術スタック (Tech Stack)
* **Language**: C++
* **Graphics API**: DirectX 12
* **Shader Language**: HLSL
* **GUI**: Dear ImGui、assimp、DirectTex
