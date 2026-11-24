using System;
using System.Collections.Generic;
using System.Linq;

namespace ContactBook;

public sealed class SearchHit
{
    public Person Person { get; init; } = null!;
    public string Snippet { get; init; } = "";
    public int Score { get; init; }
}

/// <summary>模糊搜索：名字 / 电话 / 邮箱 / 地址 / 分组。空格分隔的多个关键词需同时命中。</summary>
public static class SearchEngine
{
    public static List<SearchHit> Search(IEnumerable<Person> people, string query, int max = 40)
    {
        var q = (query ?? "").Trim();
        if (q.Length == 0) return new List<SearchHit>();

        var tokens = q.Split(new[] { ' ', '\t', '，', ',' }, StringSplitOptions.RemoveEmptyEntries);
        if (tokens.Length == 0) return new List<SearchHit>();

        var hits = new List<SearchHit>();
        foreach (var person in people)
        {
            var fields = BuildFields(person);
            int total = 0;
            bool all = true;
            string snippet = "";
            int bestFieldScore = -1;

            foreach (var token in tokens)
            {
                int best = -1;
                string bestText = "";
                string bestLabel = "";
                foreach (var (label, text) in fields)
                {
                    int s = MatchScore(text, token);
                    if (s > best)
                    {
                        best = s;
                        bestText = text;
                        bestLabel = label;
                    }
                }

                if (best < 0)
                {
                    all = false;
                    break;
                }

                total += best;
                if (best > bestFieldScore || snippet.Length == 0)
                {
                    bestFieldScore = best;
                    snippet = $"{bestLabel} · {bestText}";
                }
            }

            if (!all) continue;
            // 名字命中额外加权，让"搜人名"优先出现。
            if (MatchScore(person.DisplayName, tokens[0]) >= 0) total += 40;
            hits.Add(new SearchHit { Person = person, Snippet = snippet, Score = total });
        }

        return hits.OrderByDescending(h => h.Score)
                   .ThenBy(h => h.Person, Comparer<Person>.Create(NameSort.Compare))
                   .Take(max)
                   .ToList();
    }

    private static List<(string Label, string Text)> BuildFields(Person p)
    {
        var fields = new List<(string, string)> { ("名字", p.DisplayName) };
        foreach (var phone in p.Phones) fields.Add(("电话", phone));
        foreach (var email in p.Emails) fields.Add(("邮箱", email));
        foreach (var addr in p.Addresses) fields.Add(("地址", addr.Display));
        foreach (var g in p.Groups) fields.Add(("分组", g));
        return fields;
    }

    /// <summary>返回匹配得分，-1 表示不匹配。连续包含得分最高，其次是字符顺序出现（子序列）。</summary>
    private static int MatchScore(string text, string token)
    {
        if (string.IsNullOrEmpty(text)) return -1;
        int idx = text.IndexOf(token, StringComparison.OrdinalIgnoreCase);
        if (idx >= 0)
        {
            return 100 - Math.Min(idx, 30) + Math.Min(token.Length, 12) * 2;
        }

        // 子序列匹配：a..b..c 依次出现即可（类似 fzf 的模糊匹配）。
        int ti = 0;
        int gaps = 0;
        for (int i = 0; i < text.Length && ti < token.Length; i++)
        {
            if (char.ToLowerInvariant(text[i]) == char.ToLowerInvariant(token[ti]))
            {
                ti++;
            }
            else if (ti > 0)
            {
                gaps++;
            }
        }

        if (ti < token.Length) return -1;
        return Math.Max(5, 50 - gaps);
    }
}
