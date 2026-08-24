# GitHubへ初回pushする手順

## 1. ローカル設定を準備

`include/secrets.example.h`を`include/secrets.h`へコピーして編集します。
`secrets.h`は`.gitignore`対象なので、Wi-Fi認証情報はリポジトリへ入りません。

## 2. ローカルGitリポジトリを作成

PowerShellでChainOSCPadプロジェクトのルートへ移動して実行します。

```powershell
git init -b main
git status
git add .
git status
git commit -m "Initial ChainOSCPad hardware test firmware"
```

2回目の`git status`で、`include/secrets.h`がコミット対象に表示されていないことを
必ず確認します。

## 3. GitHub側に空のリポジトリを作成

GitHubの`New repository`画面で、リポジトリ名を`ChainOSCPad`として作成します。
ローカル側にREADMEと`.gitignore`があるため、GitHub側の`Add a README file`、
`.gitignore`、Licenseはこの時点では追加せず、空のリポジトリとして作成します。

公開範囲は用途に応じてPublicまたはPrivateを選択します。

## 4. remoteを登録してpush

`YOUR_GITHUB_NAME`を自分のGitHubユーザー名へ置き換えます。

```powershell
git remote add origin https://github.com/YOUR_GITHUB_NAME/ChainOSCPad.git
git remote -v
git push -u origin main
```

HTTPS認証でパスワードを求められた場合、GitHubアカウントの通常パスワードではなく、
Git Credential Manager、ブラウザー認証、またはPersonal Access Tokenを使用します。

## GitHub CLIを使う場合

GitHub CLIをインストールしてログイン済みなら、GitHubのWeb画面で先にリポジトリを
作らず、初回コミット後に次の1行で作成とpushを実行できます。

```powershell
gh repo create ChainOSCPad --public --source=. --remote=origin --push
```

非公開にする場合は`--public`を`--private`へ変更します。

## 以後の更新

```powershell
git status
git add .
git commit -m "Describe the change"
git push
```

機能単位で小さくコミットすると、問題が起きた変更を追跡しやすくなります。

